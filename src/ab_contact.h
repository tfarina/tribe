#ifndef AB_CONTACT_H
#define AB_CONTACT_H

typedef struct ab_contact_s
{
  int id;
  char *fname;
  char *lname;
  char *email;
} ab_contact_t;

ab_contact_t *ab_contact_new(void);
void          ab_contact_free(ab_contact_t *contact);

int         ab_contact_get_id(ab_contact_t *contact);
void        ab_contact_set_id(ab_contact_t *contact, int id);

char const *ab_contact_get_first_name(ab_contact_t *contact);
void        ab_contact_set_first_name(ab_contact_t *contact, char const *fname);

char const *ab_contact_get_last_name(ab_contact_t *contact);
void        ab_contact_set_last_name(ab_contact_t *contact, char const *lname);

char const *ab_contact_get_email(ab_contact_t *contact);
void        ab_contact_set_email(ab_contact_t *contact, char const *email);

#endif /* AB_CONTACT_H */
