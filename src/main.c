#include <gtk/gtk.h>

#include "ab.h"
#include "dirs.h"
#include "tribe-application.h"

int
main(int argc, char **argv)
{
  int rc;
  int status = 0; /* success */
  char const *dbdir;
  TribeApplication *app;
  GtkWidget *window;

  if (!dirs_init())
  {
    return 1;
  }

  dbdir = dirs_get_user_data_dir();

  rc = ab_init(dbdir);
  if (rc < 0)
  {
    status = 1;
    goto out;
  }

  gtk_init(&argc, &argv);

  app = tribe_application_new();
  window = tribe_application_create_main_window(app);

  /* Show main window as late as possible. */
  gtk_widget_show_all(window);

  gtk_main();

  g_object_unref(app);

out:
  ab_fini();

  return status;
}
