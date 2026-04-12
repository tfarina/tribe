#include "commands.h"

#include <stdlib.h>

#include "ab.h"
#include "common.h"
#include "dirs.h"

int cmd_list(int argc, char **argv) {
  int rc;
  int status = 0;
  char const *dbdir;
  int num_contacts = 0;
  ABContact **contacts = NULL;
  int i;

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

  rc = ab_enum_contacts_v2(&contacts, &num_contacts);
  if (rc < 0) {
    status = 1;
    goto out;
  }

  rc = print_contact_list_v2(contacts, num_contacts);

out:
  if (contacts) {
    for (i = 0; i < num_contacts; i++) {
      ab_contact_free(contacts[i]);
    }
    free(contacts);
    contacts = NULL;
  }

  ab_fini();
  dirs_shutdown();

  return status;
}
