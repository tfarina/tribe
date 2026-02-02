#include "dirs.h"

#include <glib.h>

#include "user.h"

static char *g_user_data_dir;

void
dirs_init(void)
{
  g_user_data_dir = g_build_filename(user_data_dir(), "tribe", NULL);
}

char *
dirs_get_user_data_dir(void)
{
  return g_user_data_dir;
}
