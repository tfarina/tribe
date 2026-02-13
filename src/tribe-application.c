#include "tribe-application.h"

#include "window.h"

struct _TribeApplicationPrivate
{
};

G_DEFINE_TYPE (TribeApplication, tribe_application, G_TYPE_OBJECT)

static void
on_destroy_cb (GtkWidget *widget,
               gpointer   user_data)
{
  gtk_main_quit();
}

static gboolean
on_delete_event_cb (GtkWidget *widget,
                    GdkEvent  *event,
                    gpointer   user_data)
{
  return FALSE;
}

static void
tribe_application_finalize (GObject *object)
{
  TribeApplication *self = TRIBE_APPLICATION (object);

  G_OBJECT_CLASS (tribe_application_parent_class)->finalize (object);
}

static void
tribe_application_class_init (TribeApplicationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = tribe_application_finalize;

  g_type_class_add_private (klass, sizeof (TribeApplicationPrivate));
}

static void
tribe_application_init (TribeApplication *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, TRIBE_TYPE_APPLICATION, TribeApplicationPrivate);
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

  main_window = create_main_window ();

  g_signal_connect(G_OBJECT(main_window), "destroy",
                   G_CALLBACK(on_destroy_cb), NULL);
  g_signal_connect(G_OBJECT(main_window), "delete-event",
                   G_CALLBACK(on_delete_event_cb), NULL);

  return main_window;
}
