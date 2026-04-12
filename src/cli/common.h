#ifndef COMMON_H
#define COMMON_H

#include <glib.h>

#include "ab_contact.h"

/**
 * Prints to standard output all the contacts found in the database.
 */
int print_contact_list(GList *contact_list);

/**
 * Prints to standard output all the contacts found in the database.
 */
int print_contact_list_v2(ABContact **contacts_list, int count);

#endif /* COMMON_H */
