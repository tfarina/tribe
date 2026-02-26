#ifndef CONTACT_EDITOR_H
#define CONTACT_EDITOR_H

#include <gtk/gtk.h>

#include "ab.h"

typedef enum {
  TRIBE_CONTACT_EDITOR_MODE_CREATE,
  TRIBE_CONTACT_EDITOR_MODE_EDIT,
} TribeContactEditorMode;

typedef void (*editor_post_cb_t)(ab_contact_t *contact, gpointer user_data);

void contact_editor_new(GtkWindow *parent, TribeContactEditorMode mode, ab_contact_t *contact, editor_post_cb_t post_cb, gpointer user_data);

#endif /* CONTACT_EDITOR_H */
