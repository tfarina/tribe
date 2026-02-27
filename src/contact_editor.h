#ifndef CONTACT_EDITOR_H
#define CONTACT_EDITOR_H

#include <gtk/gtk.h>

#include "ab.h"

typedef enum {
  TRIBE_CONTACT_EDITOR_MODE_CREATE,
  TRIBE_CONTACT_EDITOR_MODE_EDIT,
} TribeContactEditorMode;

typedef void (*TribeContactEditorResponseFunc)(GtkWidget    *editor,
					       ab_contact_t *contact,
					       gpointer      user_data);

void contact_editor_new(GtkWindow                      *parent,
			ab_contact_t                   *contact,
			TribeContactEditorMode          mode,
			TribeContactEditorResponseFunc  response_cb,
			gpointer                        user_data);

#endif /* CONTACT_EDITOR_H */
