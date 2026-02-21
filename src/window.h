#ifndef WINDOW_H
#define WINDOW_H

#include <gtk/gtk.h>

#include "tribe-application.h"

G_BEGIN_DECLS

#define TRIBE_TYPE_WINDOW            (tribe_window_get_type ())
#define TRIBE_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRIBE_TYPE_WINDOW, TribeWindow))
#define TRIBE_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TRIBE_TYPE_WINDOW, TribeWindowClass))
#define TRIBE_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRIBE_TYPE_WINDOW))
#define TRIBE_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TRIBE_TYPE_WINDOW))
#define TRIBE_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRIBE_TYPE_WINDOW, TribeWindowClass))

typedef struct _TribeWindow        TribeWindow;
typedef struct _TribeWindowClass   TribeWindowClass;
typedef struct _TribeWindowPrivate TribeWindowPrivate;

struct _TribeWindow
{
  GtkWindow            parent;
  TribeWindowPrivate  *priv;
};

struct _TribeWindowClass
{
  GtkWindowClass       parent_class;
};

GType               tribe_window_get_type                (void) G_GNUC_CONST;

GtkWidget *         tribe_window_new                     (TribeApplication *application);

GtkWidget *create_main_window(void);

G_END_DECLS

#endif /* WINDOW_H */
