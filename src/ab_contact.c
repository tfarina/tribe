#include "ab_contact.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

int ab_contact_create(ab_contact_t **pp_contact) {
  ab_contact_t *contact = NULL;
  int rc = 0; /* success */

  /* Allocate space for the ab_contact structure
   */
  contact = malloc(sizeof(ab_contact_t));
  if (!contact) {
    rc = -ENOMEM;
    goto err;
  }

  /* Zero init the object
   */
  memset(contact, 0, sizeof(ab_contact_t));

  contact->fname = NULL;
  contact->lname = NULL;
  contact->email = NULL;

  *pp_contact = contact;

  return 0;

err:
  free(contact);
  contact = NULL;

  return rc;
}

void ab_contact_destroy(ab_contact_t *contact) {
  if (!contact) {
    return;
  }

  free(contact->fname);
  contact->fname = NULL;
  free(contact->lname);
  contact->lname = NULL;
  free(contact->email);
  contact->email = NULL;

  free(contact);
  contact = NULL;
}

char const *ab_contact_get_first_name(ab_contact_t *contact) {
  return contact->fname;
}

void ab_contact_set_first_name(ab_contact_t *contact, char const *fname) {
  g_free(contact->fname);
  contact->fname = g_strdup(fname);
}

char const *ab_contact_get_last_name(ab_contact_t *contact) {
  return contact->lname;
}

void ab_contact_set_last_name(ab_contact_t *contact, char const *lname) {
  g_free(contact->lname);
  contact->lname = g_strdup(lname);
}

char const *ab_contact_get_email(ab_contact_t *contact) {
  return contact->email;
}

void ab_contact_set_email(ab_contact_t *contact, char const *email) {
  g_free(contact->email);
  contact->email = g_strdup(email);
}
