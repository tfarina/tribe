#include "ab_contact.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

ABContact* ab_contact_new(void) {
  ABContact *contact;

  contact = g_new0(ABContact, 1);

  return contact;
}

void ab_contact_free(ABContact *contact) {
  if (!contact) {
    return;
  }

  g_free(contact->fname);
  g_free(contact->lname);
  g_free(contact->email);

  g_free(contact);
}

int ab_contact_get_id(ABContact *contact) {
  return contact->id;
}

void ab_contact_set_id(ABContact *contact, int id) {
  contact->id = id;
}

char const *ab_contact_get_first_name(ABContact *contact) {
  return contact->fname;
}

void ab_contact_set_first_name(ABContact *contact, char const *fname) {
  g_free(contact->fname);
  contact->fname = g_strdup(fname);
}

char const *ab_contact_get_last_name(ABContact *contact) {
  return contact->lname;
}

void ab_contact_set_last_name(ABContact *contact, char const *lname) {
  g_free(contact->lname);
  contact->lname = g_strdup(lname);
}

char const *ab_contact_get_email(ABContact *contact) {
  return contact->email;
}

void ab_contact_set_email(ABContact *contact, char const *email) {
  g_free(contact->email);
  contact->email = g_strdup(email);
}
