#ifndef AB_CONTACT_H
#define AB_CONTACT_H

typedef struct _ABContact ABContact;

typedef struct _ABContactArray
{
  int num_elements;
  ABContact **elements;
} ABContactArray;

ABContact *ab_contact_new(void);
void       ab_contact_free(ABContact *contact);

int         ab_contact_get_id(ABContact *contact);
void        ab_contact_set_id(ABContact *contact, int id);

char const *ab_contact_get_first_name(ABContact *contact);
void        ab_contact_set_first_name(ABContact *contact, char const *fname);

char const *ab_contact_get_last_name(ABContact *contact);
void        ab_contact_set_last_name(ABContact *contact, char const *lname);

char const *ab_contact_get_email(ABContact *contact);
void        ab_contact_set_email(ABContact *contact, char const *email);

#endif /* AB_CONTACT_H */
