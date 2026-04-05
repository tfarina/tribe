#include "ab_contact.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

ab_contact_t* ab_contact_new(void) {
  ab_contact_t *contact;

  contact = g_new0(ab_contact_t, 1);

  return contact;
}

void ab_contact_free(ab_contact_t *contact) {
  if (!contact) {
    return;
  }

  g_free(contact->fname);
  g_free(contact->lname);
  g_free(contact->email);

  g_free(contact);
}

int ab_contact_get_id(ab_contact_t *contact) {
  return contact->id;
}

void ab_contact_set_id(ab_contact_t *contact, int id) {
  contact->id = id;
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
