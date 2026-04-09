#ifndef AB_H_
#define AB_H_

#include "ab_contact.h"

int ab_init(char const *db_dir);
int ab_fini(void);

int ab_enum_contacts_v2(int *num_contacts, ABContact **contacts_dst);

int ab_add_contact(ABContact *contact);
int ab_add_contact_v2(ABContact *contact);

int ab_update_contact(ABContact *contact);

int ab_delete_contact(ABContact *contact);
int ab_delete_contact_v2(int id);

int ab_get_contact_by_id(int id, ABContact **pp_contact);

#endif  /* AB_H_ */
