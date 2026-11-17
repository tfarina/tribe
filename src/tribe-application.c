#include "tribe-application.h"

#include "tribe-window.h"

G_DEFINE_TYPE (TribeApplication, tribe_application, G_TYPE_OBJECT)

static void
on_destroy_cb (GtkWidget *widget,
               gpointer   user_data)
{
  (void) widget;
  (void) user_data;

  gtk_main_quit();
}

static void
tribe_application_class_init (TribeApplicationClass *klass)
{
  (void) klass;
}

static void
tribe_application_init (TribeApplication *self)
{
  (void) self;
}

TribeApplication*
tribe_application_new (void)
{
  return g_object_new (TRIBE_TYPE_APPLICATION, NULL);
}

GtkWidget*
tribe_application_create_main_window (TribeApplication *self)
{
  GtkWidget *main_window;

  g_return_val_if_fail (TRIBE_IS_APPLICATION (self), NULL);

  main_window = tribe_window_new (self);

  g_signal_connect(G_OBJECT(main_window), "destroy",
                   G_CALLBACK(on_destroy_cb), NULL);

  return main_window;
}
