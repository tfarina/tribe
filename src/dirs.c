#include "dirs.h"

#include <glib.h>

static char *g_user_data_dir;

static int
ensure_data_dir(void)
{
  if (!g_file_test(g_user_data_dir, G_FILE_TEST_IS_DIR))
  {
    if (g_mkdir_with_parents(g_user_data_dir, 0700) < 0)
    {
      return 0;
    }
  }

  return 1;
}

int
dirs_init(void)
{
  g_user_data_dir = g_build_filename(g_get_user_data_dir(), "tribe", NULL);

  if (!ensure_data_dir())
  {
    return 0;
  }

  return 1;
}

char *
dirs_get_user_data_dir(void)
{
  return g_user_data_dir;
}
