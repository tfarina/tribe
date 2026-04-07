#ifndef TRIBE_CONTACT_DIALOG_H
#define TRIBE_CONTACT_DIALOG_H

#include <gtk/gtk.h>

#include "ab.h"

typedef enum {
  TRIBE_CONTACT_DIALOG_MODE_CREATE,
  TRIBE_CONTACT_DIALOG_MODE_EDIT,
} TribeContactDialogMode;

typedef void (*TribeContactDialogResponseFunc)(GtkWidget    *dialog,
					       ABContact    *contact,
					       gpointer      user_data);

GtkWidget* tribe_contact_dialog_new(GtkWindow                      *parent,
				    ABContact                      *contact,
				    TribeContactDialogMode          mode,
				    TribeContactDialogResponseFunc  response_cb,
				    gpointer                        user_data);

#endif /* TRIBE_CONTACT_DIALOG_H */
