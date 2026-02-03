#include "util.h"

#include <glib.h>

#include "dirs.h"

int ensure_data_dir(void) {
  char const *data_dir;
  int retval;

  data_dir = dirs_get_user_data_dir();
  if (!g_file_test(data_dir, G_FILE_TEST_IS_DIR)) {
    retval = g_mkdir_with_parents(data_dir, 0700);
    if (retval < 0) {
      return 0;
    }
  }

  return 1;
}
