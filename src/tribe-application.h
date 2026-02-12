#ifndef TRIBE_APPLICATION_H
#define TRIBE_APPLICATION_H

#include <gtk/gtk.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define TRIBE_TYPE_APPLICATION            (tribe_application_get_type ())
#define TRIBE_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRIBE_TYPE_APPLICATION, TribeApplication))
#define TRIBE_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TRIBE_TYPE_APPLICATION, TribeApplicationClass))
#define TRIBE_IS_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRIBE_TYPE_APPLICATION))
#define TRIBE_IS_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TRIBE_TYPE_APPLICATION))
#define TRIBE_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRIBE_TYPE_APPLICATION, TribeApplicationClass))

typedef struct _TribeApplication        TribeApplication;
typedef struct _TribeApplicationClass   TribeApplicationClass;
typedef struct _TribeApplicationPrivate TribeApplicationPrivate;

struct _TribeApplication
{
  GObject                  parent;
  TribeApplicationPrivate *priv;
};

struct _TribeApplicationClass
{
  GObjectClass parent_class;
};

GType             tribe_application_get_type           (void) G_GNUC_CONST;

TribeApplication* tribe_application_new                (void);

GtkWidget*        tribe_application_create_main_window (TribeApplication *self);

G_END_DECLS

#endif /* TRIBE_APPLICATION_H */
