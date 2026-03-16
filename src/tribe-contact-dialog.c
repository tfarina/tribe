#include "tribe-contact-dialog.h"

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include <string.h>

#define DIALOG_DATA_KEY "dialog-data"

typedef struct
{
  ab_contact_t *contact;
  TribeContactDialogMode mode;
  TribeContactDialogResponseFunc response_func;
  gpointer window_data;

  GtkWidget *fname_entry;
  GtkWidget *lname_entry;
  GtkWidget *email_entry;
} DialogData;

static void
tribe_contact_dialog_response_cb(GtkDialog *dialog,
				 gint       response_id,
				 gpointer   user_data)
{
  DialogData *data;
  ab_contact_t *contact;

  data = g_object_get_data(G_OBJECT(dialog), DIALOG_DATA_KEY);

  switch (response_id)
  {
    case GTK_RESPONSE_OK:
      /* Determine contact object */
      if (data->mode == TRIBE_CONTACT_DIALOG_MODE_CREATE)
      {
	ab_contact_create(&contact);
      }
      else
      {
	contact = data->contact;
      }

      ab_contact_set_first_name(contact,
				gtk_entry_get_text(GTK_ENTRY(data->fname_entry)));

      ab_contact_set_last_name(contact,
			       gtk_entry_get_text(GTK_ENTRY(data->lname_entry)));

      ab_contact_set_email(contact,
			   gtk_entry_get_text(GTK_ENTRY(data->email_entry)));

      if (data->mode == TRIBE_CONTACT_DIALOG_MODE_CREATE)
      {
	ab_add_contact(contact);
      }
      else
      {
	ab_update_contact(contact);
      }

      if (data->response_func)
      {
	data->response_func(GTK_WIDGET(dialog), contact, data->window_data);
      }

      break;

    case GTK_RESPONSE_CANCEL:
    case GTK_RESPONSE_DELETE_EVENT:
      /* Nothing to do */
      break;

    default:
      break;
    }

  gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void
tribe_contact_dialog_map_cb(GtkWidget *widget, gpointer user_data)
{
  GtkWidget *fname_entry = user_data;

  gtk_widget_grab_focus(fname_entry);
}

GtkWidget*
tribe_contact_dialog_new(GtkWindow                      *parent,
			 ab_contact_t                   *contact,
			 TribeContactDialogMode          mode,
			 TribeContactDialogResponseFunc  response_cb,
			 gpointer                        user_data)
{
  DialogData *data;
  GtkWidget *dialog;
  GtkWidget *content_area;
  GtkWidget *notebook;
  GtkWidget *table;
  GtkWidget *label;
  char const* entry_text;

  data = g_new0(DialogData, 1);

  data->contact = contact;
  data->mode = mode;
  data->response_func = response_cb;
  data->window_data = user_data;

  dialog = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(dialog), "Properties");
  gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_default_size(GTK_WINDOW(dialog), 400, -1);
  g_signal_connect(dialog, "response",
		   G_CALLBACK(tribe_contact_dialog_response_cb),
		   NULL);

  content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

  notebook = gtk_notebook_new();
  gtk_container_set_border_width(GTK_CONTAINER(notebook), 6);

  gtk_box_pack_start(GTK_BOX(content_area), notebook, TRUE, TRUE, 0);

  /* Create the table to host the text entry widgets. */
  table = gtk_table_new(3 /* rows */, 2 /* columns */, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(table), 6);
  gtk_table_set_col_spacings(GTK_TABLE(table), 12);
  gtk_container_set_border_width(GTK_CONTAINER(table), 12);

  /* First row. */
  label = gtk_label_new("First:");
  gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL, 0, 0, 0);

  data->fname_entry = gtk_entry_new();
  gtk_entry_set_activates_default(GTK_ENTRY(data->fname_entry), TRUE);
  gtk_table_attach(GTK_TABLE(table), data->fname_entry, 1, 2, 0, 1,
		   GTK_EXPAND | GTK_FILL, 0, 0, 0);

  /* Second row. */
  label = gtk_label_new("Last:");
  gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL, 0, 0, 0);

  data->lname_entry = gtk_entry_new();
  gtk_entry_set_activates_default(GTK_ENTRY(data->lname_entry), TRUE);
  gtk_table_attach(GTK_TABLE(table), data->lname_entry, 1, 2, 1, 2,
		   GTK_EXPAND | GTK_FILL, 0, 0, 0);

  /* Third row. */
  label = gtk_label_new("Email:");
  gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
		   GTK_FILL, 0, 0, 0);

  data->email_entry = gtk_entry_new();
  gtk_entry_set_activates_default(GTK_ENTRY(data->email_entry), TRUE);
  gtk_table_attach(GTK_TABLE(table), data->email_entry, 1, 2, 2, 3,
		   GTK_EXPAND | GTK_FILL, 0, 0, 0);

  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table,
			   gtk_label_new("Name"));

  gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			 "OK", GTK_RESPONSE_OK,
			 "Cancel", GTK_RESPONSE_CANCEL,
			 NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

  g_signal_connect(dialog, "map",
		   G_CALLBACK(tribe_contact_dialog_map_cb),
		   data->fname_entry);

  g_object_set_data_full(G_OBJECT(dialog),
			 DIALOG_DATA_KEY,
			 data,
			 g_free);

  if (data->contact)
  {
    entry_text = ab_contact_get_first_name(data->contact);
    if (entry_text)
    {
      gtk_entry_set_text(GTK_ENTRY(data->fname_entry), entry_text);
    }

    entry_text = ab_contact_get_last_name(data->contact);
    if (entry_text)
    {
      gtk_entry_set_text(GTK_ENTRY(data->lname_entry), entry_text);
    }

    entry_text = ab_contact_get_email(data->contact);
    if (entry_text)
    {
      gtk_entry_set_text(GTK_ENTRY(data->email_entry), entry_text);
    }
  }

  return dialog;
}
