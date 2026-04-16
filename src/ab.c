#include "ab.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "third_party/sqlite/sqlite3.h"

/* Database schema version.
 *
 * This constant corresponds to PRAGMA user_version and represents the latest
 * supported database schema version. It is compared against the stored
 * user_version to determine whether schema migrations are required.
 *
 * Increment this when making incompatible changes to the database schema and
 * provide a corresponding migration step.
 */
#define DB_VERSION 0

static sqlite3 *hdb = NULL;  /* SQLite db handle */

static int _db_close(void) {
  int rc;
  int scode = 0; /* success */

  rc = sqlite3_close(hdb);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: Unable to close the database: %s\n",
	    sqlite3_errmsg(hdb));
    scode = -1;
  }
  hdb = NULL;

  return scode;
}

/**
 * Makes sure the 'contacts' table is created if it does not exist yet.
 *
 * @return return 0 on success, -1 otherwise.
 */
static int _db_init_schema(void) {
  int rc;
  char const create_sql[] =
    "CREATE TABLE IF NOT EXISTS contacts ("
    "  id INTEGER PRIMARY KEY,"     /* id */
    "  fname TEXT,"                 /* first name */
    "  lname TEXT,"                 /* last name */
    "  email TEXT"                  /* email */
    ");";
  char *err_msg = NULL;

  rc = sqlite3_exec(hdb, create_sql, 0, 0, &err_msg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_exec failed: %s\n", err_msg);
    sqlite3_free(err_msg);
    return -1;
  }

  return rc;
}

static int _db_check_integrity(void) {
  int rc;
  int scode = 0; /* success */
  sqlite3_stmt *stmt = NULL;
  unsigned char const *result = NULL;

  rc = sqlite3_prepare_v2(hdb, "PRAGMA integrity_check;", -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_ROW) {
    fprintf(stderr, "ERROR: sqlite3_step failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  result = sqlite3_column_text(stmt, 0);
  if (!result || strcmp((char const *)result, "ok") != 0) {
    fprintf(stderr,
	    "ERROR: SQLite integrity check failed: %s\n",
	    result ? (char const *)result : "(null)");
    scode = -1;
    goto out;
  }

out:
  sqlite3_finalize(stmt);

  return scode;
}

static int _db_get_user_version(int *version) {
  int rc;
  int scode = 0; /* success */
  sqlite3_stmt *stmt = NULL;

  if (!version) {
    return -EINVAL;  /* Invalid args */
  }

  rc = sqlite3_prepare_v2(hdb, "PRAGMA user_version;", -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_ROW) {
    fprintf(stderr, "ERROR: sqlite3_step failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  *version = sqlite3_column_int(stmt, 0);

out:
  sqlite3_finalize(stmt);

  return scode;
}

int ab_init(char const *db_dir) {
  int rc;
  char const db_file_name[] = "abdb.sqlite3";
  char *db_file_path;
  int user_version = 0;

  /* Do nothing if the database handle has been set. */
  if (hdb) {
    return 0;
  }

  db_file_path = g_build_filename(db_dir, db_file_name, NULL);

  rc = sqlite3_open(db_file_path, &hdb);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: Unable to open the database at %s: %s\n",
	    db_file_path, sqlite3_errmsg(hdb));
    g_free(db_file_path);
    sqlite3_close(hdb);
    hdb = NULL;
    return -1;
  }
  g_free(db_file_path);

  rc = _db_check_integrity();
  if (rc < 0) {
    sqlite3_close(hdb);
    hdb = NULL;
    return -1;
  }

  rc = _db_get_user_version(&user_version);
  if (rc < 0) {
    fprintf(stderr, "ERROR: _db_get_user_version failed\n");
    sqlite3_close(hdb);
    hdb = NULL;
    return -1;
  }

  rc = _db_init_schema();
  if (rc < 0) {
    sqlite3_close(hdb);
    hdb = NULL;
    return -1;
  }

  return 0;
}

int ab_fini(void) {
  int rc;

  rc = _db_close();
  if (rc < 0) {
    return -1;
  }

  return 0;
}

static int _db_get_contacts_row_count(int *row_count) {
  int rc;
  int scode = 0; /* success */
  char const count_sql[] = "SELECT COUNT(*) FROM contacts";
  sqlite3_stmt *count_stmt = NULL;

  if (!row_count) {
    return -EINVAL;  /* Invalid args */
  }

  rc = sqlite3_prepare_v2(hdb, count_sql, -1, &count_stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  rc = sqlite3_step(count_stmt);
  if (rc != SQLITE_ROW) {
    fprintf(stderr, "ERROR: sqlite3_step failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  *row_count = sqlite3_column_int(count_stmt, 0);

out:
  sqlite3_finalize(count_stmt);

  return scode;
}

int _db_enum_contacts(ABContactArray **contacts) {
  int rc;
  int scode = 0;
  int row_count = 0;
  char const select_sql[] = "SELECT * FROM contacts";
  sqlite3_stmt *select_stmt = NULL;
  ABContactArray *contacts_array = NULL;
  int i;

  if (!contacts) {
    return -EINVAL;  /* Invalid args */
  }

  rc = _db_get_contacts_row_count(&row_count);
  if (rc < 0) {
    return -1;
  }

  /* Allocate the container
   */
  contacts_array = malloc(sizeof(ABContactArray));
  if (!contacts_array) {
    scode = -ENOMEM;
    goto err;
  }

  contacts_array->num_elements = 0;

  /* Allocate the contacts array
   */
  contacts_array->elements = malloc(row_count * sizeof(ABContact *));
  if (!contacts_array->elements) {
    scode = -ENOMEM;
    goto err;
  }

  /* Zero init it
   */
  memset(contacts_array->elements, 0, row_count * sizeof(ABContact *));

  rc = sqlite3_prepare_v2(hdb, select_sql, -1, &select_stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto err;
  }

  for (i = 0; i < row_count; i++) {
    rc = sqlite3_step(select_stmt);
    if (rc != SQLITE_ROW)
      break;

    ABContact *contact = ab_contact_new();
    ab_contact_set_id(contact, sqlite3_column_int(select_stmt, 0));
    ab_contact_set_first_name(contact,
			      (char const *)sqlite3_column_text(select_stmt, 1));
    ab_contact_set_last_name(contact,
			     (char const *)sqlite3_column_text(select_stmt, 2));
    ab_contact_set_email(contact,
			 (char const *)sqlite3_column_text(select_stmt, 3));

    contacts_array->elements[contacts_array->num_elements++] = contact;
  }

  *contacts = contacts_array;

err:
  if (scode < 0) {
    if (contacts_array) {
      for (i = 0; i < contacts_array->num_elements; i++) {
	ab_contact_free(contacts_array->elements[i]);
      }
      free(contacts_array->elements);
      free(contacts_array);
      contacts_array = NULL;
    }
  }
  sqlite3_finalize(select_stmt);

  return scode;
}

int ab_enum_contacts_v2(ABContactArray **contacts) {
  return _db_enum_contacts(contacts);
}

int _db_insert_contact(ABContact *contact) {
  int rc = 0;
  int scode = 0;
  char const insert_sql[] =
      "INSERT INTO contacts (fname, lname, email) VALUES (?, ?, ?)";
  sqlite3_stmt *insert_stmt = NULL;

  rc = sqlite3_prepare_v2(hdb, insert_sql, -1, &insert_stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(hdb));
    return -1;
  }

  rc = sqlite3_bind_text(insert_stmt, 1, ab_contact_get_first_name(contact), -1, SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_bind_text failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  rc = sqlite3_bind_text(insert_stmt, 2, ab_contact_get_last_name(contact), -1, SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_bind_text failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  rc = sqlite3_bind_text(insert_stmt, 3, ab_contact_get_email(contact), -1, SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_bind_text failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  rc = sqlite3_step(insert_stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "ERROR: sqlite3_step failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
  }

out:
  sqlite3_finalize(insert_stmt);

  return scode;
}

/*
 * Adds the specified contact to the address book database.
 *
 * Returns:
 *   >= 0 on success (implementation-defined, e.g., row id)
 *   < 0 on failure
 */
int ab_add_contact(ABContact *contact) {
  return _db_insert_contact(contact);
}

int _db_update_contact(ABContact *contact) {
  int rc;
  int scode = 0;
  char const update_sql[] =
      "UPDATE contacts SET fname=?, lname=?, email=? WHERE id=?";
  sqlite3_stmt *update_stmt = NULL;

  rc = sqlite3_prepare_v2(hdb, update_sql, -1, &update_stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(hdb));
    return -1;
  }

  rc = sqlite3_bind_text(update_stmt, 1, ab_contact_get_first_name(contact), -1, SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_bind_text failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  rc = sqlite3_bind_text(update_stmt, 2, ab_contact_get_last_name(contact), -1, SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_bind_text failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  rc = sqlite3_bind_text(update_stmt, 3, ab_contact_get_email(contact), -1, SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_bind_text failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  rc = sqlite3_bind_int(update_stmt, 4, ab_contact_get_id(contact));
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_bind_int failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  rc = sqlite3_step(update_stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "ERROR: sqlite3_step failed: %s\n",
	    sqlite3_errmsg(hdb));
    scode = -1;
  }

out:
  sqlite3_finalize(update_stmt);

  return scode;
}

/*
 * Updates an existing contact in the address book database.
 */
int ab_update_contact(ABContact *contact) {
  return _db_update_contact(contact);
}

static int _db_delete_contact(int id) {
  int rc;
  int scode = 0;
  char const delete_sql[] = "DELETE FROM contacts WHERE id=?";
  sqlite3_stmt *delete_stmt = NULL;
  int num_change = 0;

  rc = sqlite3_prepare_v2(hdb, delete_sql, -1, &delete_stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(hdb));
    return -1;
  }

  rc = sqlite3_bind_int(delete_stmt, 1, id);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_bind_int failed: %s\n",
	    sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  rc = sqlite3_step(delete_stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "ERROR: sqlite3_step failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  /* Check the execution result
   */
  num_change = sqlite3_changes(hdb);
  if (!num_change) {
    scode = -ENOENT;  /* No such contact exists */
  }

out:
  sqlite3_finalize(delete_stmt);

  return scode;
}

/*
 * Deletes a contact from the address book database.
 */
int ab_delete_contact(ABContact *contact) {
  return _db_delete_contact(ab_contact_get_id(contact));
}

/*
 * Retrieves a contact for the given id.
 *
 * [in] id The index of the contact in the table.
 * [out] pp_contact A pointer to a pointer to the retrieved contact.
 *
 * @return 0 if successful, -1 if it was not found.
 */
int ab_get_contact_by_id(int id, ABContact **pp_contact) {
  int rc;
  int scode = 0;
  char const query[] = "SELECT * FROM contacts WHERE id=?";
  sqlite3_stmt *stmt = NULL;
  ABContact *contact = NULL;

  rc = sqlite3_prepare_v2(hdb, query, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_prepare_v2 failed: %s\n",
            sqlite3_errmsg(hdb));
    return -1;
  }

  rc = sqlite3_bind_int(stmt, 1, id);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "ERROR: sqlite3_bind_int failed: %s\n",
	    sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
    fprintf(stderr, "ERROR: sqlite3_step failed: %s\n",
            sqlite3_errmsg(hdb));
    scode = -1;
    goto out;
  }

  /* If rc is equal to SQLITE_ROW then a contact with the given id was found! */
  if (rc == SQLITE_ROW) {
    contact = ab_contact_new();

    ab_contact_set_id(contact, sqlite3_column_int(stmt, 0));
    ab_contact_set_first_name(contact, (char const *)sqlite3_column_text(stmt, 1));
    ab_contact_set_last_name(contact, (char const *)sqlite3_column_text(stmt, 2));
    ab_contact_set_email(contact, (char const *)sqlite3_column_text(stmt, 3));

    *pp_contact = contact;
  }
  /* If rc is equal to SQLITE_DONE then NO contact with the given id was found */

out:
  sqlite3_finalize(stmt);

  return scode;
}
