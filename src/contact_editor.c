#include "contact_editor.h"

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include <string.h>

static TribeContactEditorMode current_mode;
static ab_contact_t *current_contact;
static TribeContactEditorResponseFunc response_func;
static gpointer window_data;

/*
 * Widgets
 */
static GtkWidget *fname_entry;
static GtkWidget *lname_entry;
static GtkWidget *email_entry;

static void
_on_contact_editor_ok_button_clicked_cb(GtkButton *button,
					gpointer   user_data)
{
  GtkWidget *window = user_data;
  ab_contact_t *contact;

  /* Determine contact object */
  if (current_mode == TRIBE_CONTACT_EDITOR_MODE_CREATE)
  {
    ab_contact_create(&contact);
  }
  else
  {
    contact = current_contact;
  }

  ab_contact_set_first_name(contact,
			    gtk_entry_get_text(GTK_ENTRY(fname_entry)));


  ab_contact_set_last_name(contact,
			   gtk_entry_get_text(GTK_ENTRY(lname_entry)));


  ab_contact_set_email(contact,
		       gtk_entry_get_text(GTK_ENTRY(email_entry)));


  if (current_mode == TRIBE_CONTACT_EDITOR_MODE_CREATE)
  {
    ab_add_contact(contact);
  }
  else
  {
    ab_update_contact(contact);
  }

  /* Notify caller */
  if (response_func)
  {
    response_func(window, contact, window_data);
  }

  gtk_widget_destroy(window);
}

static void
_on_contact_editor_cancel_button_clicked_cb(GtkButton *button,
					    gpointer   user_data)
{
  GtkWidget *window = user_data;

  gtk_widget_destroy(window);
}

static gboolean
_on_contact_editor_key_press_cb(GtkWidget   *widget,
				GdkEventKey *event,
				gpointer     user_data)
{
  GtkWidget *window = user_data;

  if (event && event->keyval == GDK_KEY_Escape)
  {
    gtk_widget_destroy(window);
  }
  return FALSE;
}

static void
_on_contact_editor_map_cb(GtkWidget *widget, gpointer user_data)
{
  GtkWidget *fname_entry = user_data;

  gtk_widget_grab_focus(fname_entry);
}

GtkWidget *
contact_editor_new(GtkWindow                      *parent,
		   ab_contact_t                   *contact,
		   TribeContactEditorMode          mode,
		   TribeContactEditorResponseFunc  response_cb,
		   gpointer                        user_data)
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *notebook;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *bbox;
  GtkWidget *cancel_btn;
  GtkWidget *ok_btn;
  char const* entry_text;

  current_mode = mode;
  current_contact = contact;
  response_func = response_cb;
  window_data = user_data;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Properties");
  gtk_window_set_transient_for(GTK_WINDOW(window), parent);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_modal(GTK_WINDOW(window), TRUE);
  gtk_window_set_type_hint(GTK_WINDOW(window),
			   GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 450);
  g_signal_connect(G_OBJECT(window), "key-press-event",
		   G_CALLBACK(_on_contact_editor_key_press_cb),
		   window);

  vbox = gtk_vbox_new(FALSE, 6);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  notebook = gtk_notebook_new();
  gtk_container_set_border_width(GTK_CONTAINER(notebook), 6);

  gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

  /* Create the table to host the text entry widgets. */
  table = gtk_table_new(3 /* rows */, 2 /* columns */, FALSE);

  gtk_container_set_border_width(GTK_CONTAINER(table), 4);
  gtk_table_set_row_spacings(GTK_TABLE(table), 6);
  gtk_table_set_col_spacings(GTK_TABLE(table), 12);

  /* First row. */
  label = gtk_label_new("First:");
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL, 0, 0, 0);
  gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);

  fname_entry = gtk_entry_new();

  gtk_table_attach(GTK_TABLE(table), fname_entry, 1, 2, 0, 1,
		   GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0, 0);

  /* Second row. */
  label = gtk_label_new("Last:");
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL, 0, 0, 0);
  gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);

  lname_entry = gtk_entry_new();

  gtk_table_attach(GTK_TABLE(table), lname_entry, 1, 2, 1, 2,
		   GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0, 0);

  /* Third row. */
  label = gtk_label_new("Email:");
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
		   GTK_FILL, 0, 0, 0);
  gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);

  email_entry = gtk_entry_new();

  gtk_table_attach(GTK_TABLE(table), email_entry, 1, 2, 2, 3,
		   GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0, 0);

  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table,
			   gtk_label_new("Name"));

  /*
   * Horizontal Button Box area
   */
  bbox = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
  gtk_box_set_spacing(GTK_BOX(bbox), 5);
  gtk_container_set_border_width(GTK_CONTAINER(bbox), 4);

  ok_btn = gtk_button_new_with_label("OK");
  gtk_widget_set_can_default(ok_btn, TRUE);
  gtk_box_pack_start(GTK_BOX(bbox), ok_btn, TRUE, TRUE, 0);

  cancel_btn = gtk_button_new_with_label("Cancel");
  gtk_box_pack_start(GTK_BOX(bbox), cancel_btn, TRUE, TRUE, 0);

  gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);

  /*
   * This should make sure the OK button is the default button for this dialog.
   */
  gtk_widget_grab_default(ok_btn);

  g_signal_connect(ok_btn, "clicked",
                   G_CALLBACK(_on_contact_editor_ok_button_clicked_cb),
		   window);

  g_signal_connect(cancel_btn, "clicked",
		   G_CALLBACK(_on_contact_editor_cancel_button_clicked_cb),
		   window);

  g_signal_connect(window, "map",
		   G_CALLBACK(_on_contact_editor_map_cb),
		   fname_entry);

  if (current_contact)
  {
    entry_text = ab_contact_get_first_name(current_contact);
    if (entry_text)
    {
      gtk_entry_set_text(GTK_ENTRY(fname_entry), entry_text);
    }

    entry_text = ab_contact_get_last_name(current_contact);
    if (entry_text)
    {
      gtk_entry_set_text(GTK_ENTRY(lname_entry), entry_text);
    }

    entry_text = ab_contact_get_email(current_contact);
    if (entry_text)
    {
      gtk_entry_set_text(GTK_ENTRY(email_entry), entry_text);
    }
  }

  return window;
}
