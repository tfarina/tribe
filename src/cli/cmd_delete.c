#include "commands.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "ab.h"
#include "dirs.h"

int cmd_delete(int argc, char **argv) {
  int rc;
  int status = 0;
  char const *dbdir;
  char *end;
  ABContactId id;
  ABContact *contact = NULL;

  if (argc != 2) {
    fprintf(stderr, "usage: %s <id>\n", argv[0]);
    return 1;
  }

  if (!dirs_init())
  {
    return 1;
  }

  dbdir = dirs_get_user_data_dir();

  rc = ab_init(dbdir);
  if (rc < 0) {
    status = 1;
    goto out;
  }

  errno = 0;

  id = strtoimax(argv[1], &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Error: contact id is out of range: %s\n", argv[1]);
    status = 1;
    goto out;
  }

  if (end == argv[1] || *end != '\0') {
    fprintf(stderr, "Error: invalid contact id: %s\n", argv[1]);
    status = 1;
    goto out;
  }

  rc = ab_get_contact_by_id(id, &contact);
  if (rc < 0 || !contact) {
    fprintf(stderr, "Error: no contact with id %" PRId64 " was found in our database.\n",
            id);
    status = 1;
    goto out;
  }

  rc = ab_delete_contact(contact);
  if (rc < 0) {
    status = 1;
    goto out;
  }

out:
  if (contact)
    ab_contact_free(contact);

  ab_fini();
  dirs_shutdown();

  return status;
}
