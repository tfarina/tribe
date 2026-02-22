/*https://stackoverflow.com/questions/45389291/how-to-display-data-from-a-sqlite-database-into-a-gtk-treeview*/

#include "tribe-window.h"

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "ab.h"
#include "about_dialog.h"
#include "contact_editor.h"
#include "dirs.h"

struct _TribeWindowPrivate
{
  GtkUIManager *ui_manager;
  GtkWidget    *toolbar;
  GtkToolItem  *tb_edit;
  GtkToolItem  *tb_delete;
  GtkWidget    *list_view;
  GtkWidget    *statusbar;
  guint         statusbar_cid;
};

G_DEFINE_TYPE (TribeWindow, tribe_window, GTK_TYPE_WINDOW)

#define APPTITLE "Tribe"

/* Initial window size. */
#define INIT_WINDOW_WIDTH (gdk_screen_width() * 3 / 4)
#define INIT_WINDOW_HEIGHT (gdk_screen_height() * 3 / 4)

/*
 * Constants for accessing columns in a GtkListStore.
 */
enum
{
  COL_FIRST_NAME = 0,
  COL_LAST_NAME,
  COL_EMAIL,
  COL_PTR,
  N_COLUMNS
};

static gint
contact_column_compare_func(GtkTreeModel *model,
			    GtkTreeIter *a,
			    GtkTreeIter *b,
			    gpointer data)
{

  gint sortcol = GPOINTER_TO_INT(data);
  gchar *sa, *sb;
  gint ret = 0;

  switch (sortcol) {
  case COL_FIRST_NAME:
    gtk_tree_model_get(model, a, COL_FIRST_NAME, &sa, -1);
    gtk_tree_model_get(model, b, COL_FIRST_NAME, &sb, -1);
    ret = g_utf8_collate(sa, sb);
    g_free(sa);
    g_free(sb);
    break;
  case COL_LAST_NAME:
    gtk_tree_model_get(model, a, COL_LAST_NAME, &sa, -1);
    gtk_tree_model_get(model, b, COL_LAST_NAME, &sb, -1);
    ret = g_utf8_collate(sa, sb);
    g_free(sa);
    g_free(sb);
    break;
  case COL_EMAIL:
    gtk_tree_model_get(model, a, COL_EMAIL, &sa, -1);
    gtk_tree_model_get(model, b, COL_EMAIL, &sb, -1);
    ret = g_utf8_collate(sa, sb);
    g_free(sa);
    g_free(sb);
    break;
  default:
    g_return_val_if_reached(0);
  }

  return ret;
}

/*
 * Main window widgets.
 */
typedef struct
{
  GtkWindow    *main_window;
  GtkUIManager *ui_manager;
  GtkWidget    *toolbar;
  GtkToolItem  *tb_edit;
  GtkToolItem  *tb_delete;
  GtkWidget    *list_view;
  GtkWidget    *statusbar;
  guint         statusbar_cid;
} WindowData;

/*
 * Menubar callbacks
 */

/* File menu */

static void _on_file_new_contact_cb(GtkAction *action, gpointer data);
static void _on_file_properties_cb(GtkAction *action, gpointer data);
static void _on_file_delete_cb(GtkAction *action, gpointer data);
static void _on_file_exit_cb(GtkAction *action, gpointer data);

/* Edit menu */

static void _on_edit_select_all_cb(GtkAction *action, gpointer data);

/* View menu */

static void _on_view_toolbar_cb(GtkAction *action, gpointer data);
static void _on_view_statusbar_cb(GtkAction *action, gpointer data);
static void _on_view_fullscreen_cb(GtkAction *action, gpointer data);
static void _on_view_toolbar_style_cb(GtkAction *action, GtkRadioAction *current, gpointer data);

/* Help menu */

static void _on_help_contents_cb(GtkAction *action, gpointer data);
static void _on_help_about_cb(GtkAction *action, gpointer data);

enum {
  TOOLBAR_STYLE_ICONS,
  TOOLBAR_STYLE_TEXT,
  TOOLBAR_STYLE_BOTH,
  TOOLBAR_STYLE_BOTH_HORIZ
};

static char const ui_definition[] =
  "<ui>"
    "<menubar name=\"MainMenu\">"
      "<menu action=\"FileMenu\">"
        "<menuitem action=\"NewContact\"/>"
        "<separator/>"
        "<menuitem action=\"Properties\"/>"
        "<menuitem action=\"Delete\"/>"
        "<separator/>"
        "<menuitem action=\"Exit\"/>"
      "</menu>"
      "<menu action=\"EditMenu\">"
        "<menuitem action=\"SelectAll\"/>"
      "</menu>"
      "<menu action=\"ViewMenu\">"
        "<menuitem action=\"ToolBar\"/>"
        "<menuitem action=\"StatusBar\"/>"
        "<separator/>"
        "<menu action=\"ToolBarStyleMenu\">"
	  "<menuitem action=\"TextBelowIcon\"/>"
	  "<menuitem action=\"TextBesideIcon\"/>"
	  "<menuitem action=\"IconsOnly\"/>"
	  "<menuitem action=\"TextOnly\"/>"
        "</menu>"
        "<separator/>"
        "<menuitem action=\"FullScreen\"/>"
      "</menu>"
      "<menu action=\"HelpMenu\">"
        "<menuitem action=\"Contents\"/>"
        "<separator/>"
        "<menuitem action=\"About\"/>"
      "</menu>"
    "</menubar>"
    "<popup name=\"LVMenu\">"
      "<menuitem action=\"LVNewContact\"/>"
      "<separator/>"
      "<menuitem action=\"LVProperties\"/>"
      "<menuitem action=\"LVDelete\"/>"
    "</popup>"
  "</ui>";

static GtkActionEntry menubar_entries[] =
{
  /*
   * Menus
   */
  {"FileMenu", NULL, "_File", NULL, NULL, NULL }, /* name, stock id, label */
  {"EditMenu", NULL, "_Edit", NULL, NULL, NULL },
  {"ViewMenu", NULL, "_View", NULL, NULL, NULL },
  {"HelpMenu", NULL, "_Help", NULL, NULL, NULL },

  /*
   * File menu
   */
  {"NewContact", NULL,                    /* name, stock id */
   "New _Contact...", "<control>N",       /* label, accelerator */
   NULL,                                  /* tooltip */
   G_CALLBACK(_on_file_new_contact_cb) },
  /* Separator --- */
  {"Properties", NULL,
   "P_roperties", "<alt>Return",
   NULL,
   G_CALLBACK(_on_file_properties_cb) },
  {"Delete", NULL,
   "_Delete", NULL,
   NULL,
   G_CALLBACK(_on_file_delete_cb) },
  /* Separator --- */
  {"Exit", NULL,
   "E_xit", NULL,
   NULL,
   G_CALLBACK(_on_file_exit_cb) },

  /*
   * Edit menu
   */
  {"SelectAll", NULL,
   "Select _All", "<control>A",
   NULL,
   G_CALLBACK(_on_edit_select_all_cb) },

  /*
   * View menu
   */
  {"ToolBarStyleMenu", NULL,
   "Toolbar Style", NULL,
   NULL,
   NULL },
  /* Separator --- */

  /*
   * Help menu
   */
  {"Contents", NULL,
   "_Contents", "F1",
   NULL,
   G_CALLBACK(_on_help_contents_cb) },
  /* Separator --- */
  {"About", NULL,
   "_About", NULL,
   NULL,
   G_CALLBACK(_on_help_about_cb) }
};

static GtkToggleActionEntry menubar_toggle_entries[] =
{
  /*
   * View menu
   */
  {"ToolBar", NULL,                 /* name, stock id */
   "_Toolbar", "<control><alt>T",   /* label, accelerator */
   NULL,                            /* tooltip */
   G_CALLBACK(_on_view_toolbar_cb),
   FALSE },                         /* is_active */
  {"StatusBar", NULL,
   "_Status Bar", "",
   NULL,
   G_CALLBACK(_on_view_statusbar_cb),
   FALSE },
  /* Separator --- */
  {"FullScreen", NULL,
   "_Full Screen", "F11",
   NULL,
   G_CALLBACK(_on_view_fullscreen_cb),
   FALSE },
};

static GtkRadioActionEntry menubar_radio_entries[] =
{
  {"TextBelowIcon", NULL,           /* name, stock id */
   "Text _below icons", NULL,       /* label, accelerator */
   NULL,                            /* tooltip */
   TOOLBAR_STYLE_BOTH },            /* value */
  {"TextBesideIcon", NULL,
   "Text be_side icons", NULL,
   NULL,
   TOOLBAR_STYLE_BOTH_HORIZ },
  {"IconsOnly", NULL,
   "_Icons only", NULL,
   NULL,
   TOOLBAR_STYLE_ICONS },
  {"TextOnly", NULL,
   "_Text only", NULL,
   NULL,
   TOOLBAR_STYLE_TEXT },
};

static GtkActionEntry list_context_entries[] =
{
  {"LVNewContact", NULL,                  /* name, stock id */
   "New _Contact...", NULL,               /* label, accelerator */
   NULL,                                  /* tooltip */
   G_CALLBACK(_on_file_new_contact_cb) },
  /* Separator --- */
  {"LVProperties", NULL,
   "P_roperties", NULL,
   NULL,
   G_CALLBACK(_on_file_properties_cb) },
  {"LVDelete", NULL,
   "_Delete", NULL,
   NULL,
   G_CALLBACK(_on_file_delete_cb) },
};

/*
 * Prototype declarations
 */

static void _append_item_to_list_store(TribeWindow *window, ab_contact_t *contact);
static void _on_new_contact_cb(ab_contact_t *contact, gpointer user_data);
static void _on_edit_contact_cb(ab_contact_t *contact, gpointer user_data);

static void
_edit_selection(TribeWindow *window)
{
  TribeWindowPrivate *priv;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GList *paths;
  GList *cur;
  ab_contact_t *contact;

  priv = window->priv;
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->list_view));

  paths = gtk_tree_selection_get_selected_rows(selection, &model);

  for (cur = paths; cur; cur = g_list_next(cur))
    {
      if (!gtk_tree_model_get_iter(model, &iter, cur->data))
	{
	  continue;
	}

      gtk_tree_model_get(model, &iter, COL_PTR, (ab_contact_t *)&contact, -1);
      if (!contact)
	{
	  continue;
	}

      contact_editor_new(GTK_WINDOW(window), AC_EDIT, contact, _on_edit_contact_cb, window);
    }

  g_list_free_full(paths, (GDestroyNotify)gtk_tree_path_free);
}

static void
_remove_selection(TribeWindow *window)
{
  TribeWindowPrivate *priv;
  GtkWidget *dialog;
  gint response;
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GList *paths;
  GList *references = NULL;
  GList *row;
  GtkTreeIter iter;
  ab_contact_t *contact;
  gboolean has_row = FALSE;
  gint n;

  priv = window->priv;
  dialog = gtk_message_dialog_new(GTK_WINDOW(window),
				  GTK_DIALOG_DESTROY_WITH_PARENT,
				  GTK_MESSAGE_QUESTION,
				  GTK_BUTTONS_YES_NO,
				  "Are you sure you want to delete the selected item(s)?");
  gtk_window_set_title(GTK_WINDOW(dialog), APPTITLE);
  response = gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  if (GTK_RESPONSE_YES == response)
  {
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->list_view));
    paths = gtk_tree_selection_get_selected_rows(selection, &model);

    /* The code below came mostly from:
     * https://github.com/tristanheaven/gtkhash/blob/master/src/list.c
     * https://github.com/raspberrypi-ui/pcmanfm-bullseye/blob/7393401d8d61d5c1cbd7f653b6549875055bc2e5/src/pref.c#L433
     */

    /* Convert paths to references. */
    for (row = paths; row; row = g_list_next(row))
    {
      GtkTreeRowReference *ref;

      ref = gtk_tree_row_reference_new(model, row->data);
      references = g_list_prepend(references, ref);
      gtk_tree_path_free(row->data);
    }

    g_list_free(paths);

    /* Remove rows from model. */
    for (row = references; row; row = g_list_next(row))
    {
      GtkTreeRowReference *ref = row->data;
      GtkTreePath *path = gtk_tree_row_reference_get_path(ref);

      if (gtk_tree_model_get_iter(model, &iter, path))
      {
        gtk_tree_model_get(model, &iter, COL_PTR, (ab_contact_t *)&contact, -1);
        ab_delete_contact_v2(contact->id);
        has_row = gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
      }

      gtk_tree_path_free(path);
      gtk_tree_row_reference_free(ref);
    }

    g_list_free(references);

    /* From claws-mail src/editaddress.c:edit_person_email_delete() */
    if (!has_row)
    {
      /* The removed row was the last in the list, so iter is not
       * valid. Find out if there is at least one row remaining
       * in the list, and select the last one if so.
       */
      n = gtk_tree_model_iter_n_children(model, NULL);
      if (n > 0 && gtk_tree_model_iter_nth_child(model, &iter, NULL, n-1))
      {
        /* It exists */
        has_row = TRUE;
      }
    }

    if (has_row)
    {
      gtk_tree_selection_select_iter(selection, &iter);
    }
  }
}

/*
 * Window callbacks
 */

static void
_on_size_allocate_cb(GtkWidget		*widget,
                     GtkAllocation	*allocation)
{
#if 0
  printf("%d\n", allocation->width);
  printf("%d\n", allocation->height);
#endif
}

/*
 * Menubar callbacks
 */

/*
 * File menu
 */

static void
_on_file_new_contact_cb(GtkAction *action, gpointer data)
{
  TribeWindow *window;
  ab_contact_t *contact;

  window = TRIBE_WINDOW(data);
  ab_contact_create(&contact);

  contact_editor_new(GTK_WINDOW(window), AC_ADD, contact, _on_new_contact_cb, window);
}

static void
_on_file_exit_cb(GtkAction *action, gpointer data)
{
  TribeWindow *window;

  window = TRIBE_WINDOW(data);

  gtk_widget_destroy(GTK_WIDGET(window));
}

/*
 * Edit menu
 */

static void
_on_edit_select_all_cb(GtkAction *action, gpointer data)
{
  TribeWindow *window;
  TribeWindowPrivate *priv;
  GtkTreeSelection* selection;

  window = TRIBE_WINDOW(data);
  priv = window->priv;

  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->list_view));

  gtk_tree_selection_select_all(selection);
}

static void
_on_file_properties_cb(GtkAction *action, gpointer data)
{
  TribeWindow *window;

  window = TRIBE_WINDOW(data);

  _edit_selection(window);
}

static void
_on_file_delete_cb(GtkAction *action, gpointer data)
{
  TribeWindow *window;

  window = TRIBE_WINDOW(data);

  _remove_selection(window);
}

/*
 * View menu
 */

static void
_on_view_toolbar_cb(GtkAction *action, gpointer data)
{
  TribeWindow *window;
  TribeWindowPrivate *priv;
  gboolean state;

  window = TRIBE_WINDOW(data);
  priv = window->priv;
  state = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

  gtk_widget_set_visible(priv->toolbar, state);
}

static void
_on_view_statusbar_cb(GtkAction *action, gpointer data)
{
  TribeWindow *window;
  TribeWindowPrivate *priv;
  gboolean state;

  window = TRIBE_WINDOW(data);
  priv = window->priv;
  state = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

  gtk_widget_set_visible(priv->statusbar, state);
}

static void
_on_view_fullscreen_cb(GtkAction *action, gpointer data)
{
  TribeWindow *window;

  window = TRIBE_WINDOW(data);

  if (!gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action))) {
    gtk_window_unfullscreen(GTK_WINDOW(window));
  } else {
    gtk_window_fullscreen(GTK_WINDOW(window));
  }
}

static void
_on_view_toolbar_style_cb(GtkAction *action, GtkRadioAction *current, gpointer data)
{
  TribeWindow *window;
  TribeWindowPrivate *priv;
  gint style;

  window = TRIBE_WINDOW(data);
  priv = window->priv;
  style = gtk_radio_action_get_current_value(current);

  switch (style) {
  case TOOLBAR_STYLE_ICONS:
    gtk_toolbar_set_style(GTK_TOOLBAR(priv->toolbar), GTK_TOOLBAR_ICONS);
    break;
  case TOOLBAR_STYLE_TEXT:
    gtk_toolbar_set_style(GTK_TOOLBAR(priv->toolbar), GTK_TOOLBAR_TEXT);
    break;
  case TOOLBAR_STYLE_BOTH:
    gtk_toolbar_set_style(GTK_TOOLBAR(priv->toolbar), GTK_TOOLBAR_BOTH);
    break;
  case TOOLBAR_STYLE_BOTH_HORIZ:
    gtk_toolbar_set_style(GTK_TOOLBAR(priv->toolbar), GTK_TOOLBAR_BOTH_HORIZ);
    break;
  }
}

/*
 * Help menu
 */

static void
_on_help_contents_cb(GtkAction *action, gpointer data)
{
  TribeWindow *window;
  GdkScreen *screen = NULL;
  guint32 timestamp;
  GError *error = NULL;
  GtkWidget *dialog;

  window = TRIBE_WINDOW(data);
  screen = gtk_widget_get_screen(GTK_WIDGET(window));
  timestamp = gtk_get_current_event_time();

  gtk_show_uri(screen, "help:" PACKAGE, timestamp, &error);
  if (error) {
    dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    "%s", error->message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_error_free(error);
  }
}

static void
_on_help_about_cb(GtkAction *action, gpointer data)
{
  TribeWindow *window;

  window = TRIBE_WINDOW(data);

  show_about_dialog(GTK_WINDOW(window));
}

/*
 * List view callbacks
 */

static void
_on_selection_changed_cb(GtkTreeSelection *selection, gpointer data)
{
  TribeWindow *window;
  TribeWindowPrivate *priv;
  gint num_selected;
  gboolean can_edit;
  gboolean can_delete;
  GtkAction *action;
  char *message;

  window = TRIBE_WINDOW(data);
  priv = window->priv;
  num_selected = gtk_tree_selection_count_selected_rows(selection);

  can_edit = num_selected == 1;
  can_delete = num_selected > 0;

  action = gtk_ui_manager_get_action(priv->ui_manager, "/MainMenu/FileMenu/Properties");
  gtk_action_set_sensitive(action, can_edit);
  gtk_widget_set_sensitive(GTK_WIDGET(priv->tb_edit), can_edit);

  action = gtk_ui_manager_get_action(priv->ui_manager, "/MainMenu/FileMenu/Delete");
  gtk_action_set_sensitive(action, can_delete);
  gtk_widget_set_sensitive(GTK_WIDGET(priv->tb_delete), can_delete);

  if (priv->statusbar != NULL) {
    gtk_statusbar_pop(GTK_STATUSBAR(priv->statusbar), priv->statusbar_cid);
    if (num_selected > 0) {
       message = g_strdup_printf("%d item%s selected",
				 num_selected,
				 num_selected == 1 ? "" : "s");
       gtk_statusbar_push(GTK_STATUSBAR(priv->statusbar), priv->statusbar_cid, message);
       g_free(message);
    }
  }
}

static gboolean
_on_list_button_press_cb(GtkTreeView *widget,
                         GdkEventButton *event,
                         gpointer data)
{
  TribeWindow *window;
  TribeWindowPrivate *priv;
  GtkTreeSelection *selection;
  GtkTreePath *path;
  GtkTreeViewColumn *column;
  GtkTreeIter iter;
  ab_contact_t *contact;

  window = TRIBE_WINDOW(data);
  priv = window->priv;

  if (event->window != gtk_tree_view_get_bin_window(widget)) {
    return FALSE;
  }

  /* Borrowed from https://github.com/xfce-mirror/thunar/blob/b9ed5c29b38fcb80b1bcc6c842ccdccfe0703db2/thunar/thunar-details-view.c#L665
   * Unselect all selected items if the user clicks on an empty area
   * of the list view and no modifier key is active.
   */
  if ((event->state & gtk_accelerator_get_default_mod_mask()) == 0
      && !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(priv->list_view), event->x, event->y, &path, &column, NULL, NULL)) {
    /* Get the current selection. */
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->list_view));
    gtk_tree_selection_unselect_all(selection);
  }

  /*
   * Handles the Right Click notification.
   *
   * NOTE: It happens when the user clicks an item with the right button.
   */
  if (event->button == 3 /* 3 means right button */) {
    gint num_selected;
    gboolean can_edit;
    gboolean can_delete;
    GtkAction *action;
    GtkWidget *menu;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->list_view));
    num_selected = gtk_tree_selection_count_selected_rows(selection);

    can_edit = num_selected == 1;
    can_delete = num_selected > 0;

    action = gtk_ui_manager_get_action(priv->ui_manager, "/LVMenu/LVProperties");
    gtk_action_set_sensitive(action, can_edit);

    action = gtk_ui_manager_get_action(priv->ui_manager, "/LVMenu/LVDelete");
    gtk_action_set_sensitive(action, can_delete);

    menu = gtk_ui_manager_get_widget(priv->ui_manager, "/LVMenu");
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);

    return TRUE;
  }

  /*
   * Handles the Double-Click notification.
   *
   * NOTE: It happens when the user double-clicks an item with the left button.
   *
   * Opens the object properties dialog for the selected item.
   */
  if (event->button == 1 && event->type == GDK_2BUTTON_PRESS) {
    GtkTreeModel *model;
    GList *selected_rows;

    /* Figure out which node was clicked. */
    if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(priv->list_view), event->x, event->y, &path, &column, NULL, NULL)) {
      return FALSE;
    }
    if (column == gtk_tree_view_get_column(GTK_TREE_VIEW(priv->list_view), 0)) {
      gtk_tree_path_free(path);
      return FALSE;
    }

    gtk_tree_path_free(path);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->list_view));
    selected_rows = gtk_tree_selection_get_selected_rows(selection, &model);
    if (!selected_rows) {
      return FALSE;
    }
    gtk_tree_model_get_iter(model, &iter, selected_rows->data);
    gtk_tree_model_get(model, &iter, COL_PTR, &contact, -1);
    g_list_free_full(selected_rows, (GDestroyNotify) gtk_tree_path_free);

    if (contact != NULL) {
      contact_editor_new(GTK_WINDOW(window), AC_EDIT, contact, _on_edit_contact_cb, window);
    }

    return TRUE;
  }

  return FALSE;
}

static gboolean
_on_list_key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  TribeWindow *window;

  window = TRIBE_WINDOW(data);

  if (event && event->keyval == GDK_KEY_Delete)
    {
      _remove_selection(window);
      return TRUE;
    }

  return FALSE;
}

/*
 * Contact editor callbacks
 */

static void
_on_new_contact_cb(ab_contact_t *contact, gpointer user_data)
{
  TribeWindow *window;

  window = TRIBE_WINDOW(user_data);

  _append_item_to_list_store(window, contact);
}

static void
_on_edit_contact_cb(ab_contact_t *contact, gpointer user_data)
{
  TribeWindow *window;
  TribeWindowPrivate *priv;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GList *paths;
  GList *cur;

  window = TRIBE_WINDOW(user_data);
  priv = window->priv;
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->list_view));

  paths = gtk_tree_selection_get_selected_rows(selection, &model);

  for (cur = paths; cur; cur = g_list_next(cur))
    {
      if (gtk_tree_model_get_iter(model, &iter, cur->data))
	{
	  gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			     COL_FIRST_NAME, ab_contact_get_first_name(contact),
			     COL_LAST_NAME, ab_contact_get_last_name(contact),
			     COL_EMAIL, ab_contact_get_email(contact),
			     -1);
	}

      gtk_tree_path_free(cur->data);
    }

  g_list_free(paths);
}

/*
 * Helper functions
 */

static void
_append_item_to_list_store(TribeWindow *window, ab_contact_t *contact)
{
  TribeWindowPrivate *priv;
  GtkTreeModel *model;
  GtkTreeIter iter;

  priv = window->priv;
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(priv->list_view));

  gtk_list_store_append(GTK_LIST_STORE(model), &iter);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                     COL_FIRST_NAME, ab_contact_get_first_name(contact),
                     COL_LAST_NAME, ab_contact_get_last_name(contact),
                     COL_EMAIL, ab_contact_get_email(contact),
		     COL_PTR, contact,
                     -1);
}

static GtkWidget *
_create_menubar(TribeWindow *window)
{
  TribeWindowPrivate *priv;
  GtkActionGroup *action_group;
  GtkAction *action;

  priv = window->priv;

  priv->ui_manager = gtk_ui_manager_new();

  action_group = gtk_action_group_new("MenuActions");
  gtk_action_group_set_translation_domain(action_group, NULL);
  gtk_action_group_add_actions(action_group, menubar_entries,
			       G_N_ELEMENTS(menubar_entries), window);
  gtk_action_group_add_toggle_actions(action_group, menubar_toggle_entries,
			              G_N_ELEMENTS(menubar_toggle_entries),
				      window);
  gtk_action_group_add_radio_actions(action_group, menubar_radio_entries,
				     G_N_ELEMENTS(menubar_radio_entries),
				     1, G_CALLBACK(_on_view_toolbar_style_cb),
				     window);

  gtk_action_group_add_actions(action_group, list_context_entries,
			       G_N_ELEMENTS(list_context_entries), window);
  gtk_ui_manager_insert_action_group(priv->ui_manager, action_group, 0);
  g_object_unref(action_group);

  if (!gtk_ui_manager_add_ui_from_string(priv->ui_manager, ui_definition, -1, NULL)) {
    g_error("Unable to load menu definition\n");
  }

  gtk_window_add_accel_group(GTK_WINDOW(window), gtk_ui_manager_get_accel_group(priv->ui_manager));

  action = gtk_ui_manager_get_action(priv->ui_manager, "/MainMenu/FileMenu/Properties");
  gtk_action_set_sensitive(action, FALSE);

  action = gtk_ui_manager_get_action(priv->ui_manager, "/MainMenu/FileMenu/Delete");
  gtk_action_set_sensitive(action, FALSE);

  return gtk_ui_manager_get_widget(priv->ui_manager, "/MainMenu");
}

static void
_create_toolbar(TribeWindow *window)
{
  TribeWindowPrivate *priv;
  GtkWidget* icon;
  GtkToolItem *tb_new;

  priv = window->priv;
  priv->toolbar = gtk_toolbar_new();

  gtk_orientable_set_orientation(GTK_ORIENTABLE(priv->toolbar), GTK_ORIENTATION_HORIZONTAL);
  gtk_toolbar_set_style(GTK_TOOLBAR(priv->toolbar), GTK_TOOLBAR_BOTH);

  /* New button */
  icon = gtk_image_new_from_icon_name(GTK_STOCK_NEW, GTK_ICON_SIZE_BUTTON);
  tb_new = gtk_tool_button_new(icon, "New");
  gtk_tool_item_set_tooltip_text(tb_new, "Creates a new contact.");
  gtk_tool_item_set_is_important(tb_new, TRUE);
  gtk_toolbar_insert(GTK_TOOLBAR(priv->toolbar), tb_new, -1);
  g_signal_connect(G_OBJECT(tb_new), "clicked",
		   G_CALLBACK(_on_file_new_contact_cb), window);

  /* Properties button */
  icon = gtk_image_new_from_icon_name(GTK_STOCK_EDIT, GTK_ICON_SIZE_BUTTON);
  priv->tb_edit = gtk_tool_button_new(icon, "Properties");
  gtk_tool_item_set_tooltip_text(priv->tb_edit, "Displays the properties of the selected item.");
  gtk_tool_item_set_is_important(priv->tb_edit, TRUE);
  gtk_toolbar_insert(GTK_TOOLBAR(priv->toolbar), priv->tb_edit, -1);
  g_signal_connect(G_OBJECT(priv->tb_edit), "clicked",
		   G_CALLBACK(_on_file_properties_cb), window);

  /* Delete button */
  icon = gtk_image_new_from_icon_name(GTK_STOCK_DELETE, GTK_ICON_SIZE_BUTTON);
  priv->tb_delete = gtk_tool_button_new(icon, "Delete");
  gtk_tool_item_set_tooltip_text(priv->tb_delete, "Deletes the selected items.");
  gtk_tool_item_set_is_important(priv->tb_delete, TRUE);
  gtk_toolbar_insert(GTK_TOOLBAR(priv->toolbar), priv->tb_delete, -1);
  g_signal_connect(G_OBJECT(priv->tb_delete), "clicked",
		   G_CALLBACK(_on_file_delete_cb), window);

  gtk_widget_set_sensitive(GTK_WIDGET(priv->tb_edit), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(priv->tb_delete), FALSE);
}

static void
_create_list_view(TribeWindow *window)
{
  TribeWindowPrivate *priv;
  GtkListStore *list_store;
  GtkTreeSortable *sortable;
  GtkTreeSelection *selection;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  priv = window->priv;

  /* Create the list store. */
  list_store = gtk_list_store_new(N_COLUMNS,
                                  G_TYPE_STRING,    /* First name */
                                  G_TYPE_STRING,    /* Last name */
                                  G_TYPE_STRING,    /* Email */
                                  G_TYPE_POINTER);  /* Contact pointer */

  sortable = GTK_TREE_SORTABLE(list_store);
  gtk_tree_sortable_set_sort_func(sortable, COL_FIRST_NAME,
				  contact_column_compare_func,
				  GINT_TO_POINTER(COL_FIRST_NAME), NULL);
  gtk_tree_sortable_set_sort_func(sortable, COL_LAST_NAME,
				  contact_column_compare_func,
				  GINT_TO_POINTER(COL_LAST_NAME), NULL);
  gtk_tree_sortable_set_sort_func(sortable, COL_EMAIL,
				  contact_column_compare_func,
				  GINT_TO_POINTER(COL_EMAIL), NULL);

  /* Create the list view. */
  priv->list_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
  g_object_unref(list_store);

  /* Disable interactive search. */
  gtk_tree_view_set_enable_search(GTK_TREE_VIEW(priv->list_view), FALSE);

  /* Set selection properties. */
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->list_view));
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

  /* Set up notification callbacks. */
  g_signal_connect(selection, "changed",
		   G_CALLBACK(_on_selection_changed_cb), window);
  g_signal_connect(priv->list_view, "button-press-event",
		   G_CALLBACK(_on_list_button_press_cb), window);
  g_signal_connect(priv->list_view, "key-press-event",
		   G_CALLBACK(_on_list_key_press_cb), window);

  /* Create the columns. */
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("First Name",
                                                    renderer, "text",
						    COL_FIRST_NAME, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_column_set_sort_column_id(column, COL_FIRST_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW(priv->list_view), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Last Name",
                                                    renderer, "text",
						    COL_LAST_NAME, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_column_set_sort_column_id(column, COL_LAST_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW(priv->list_view), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Email",
                                                    renderer, "text",
						    COL_EMAIL, NULL);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_column_set_sort_column_id(column, COL_EMAIL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(priv->list_view), column);
}

static void
_populate_list_view(TribeWindow *window, GList *list)
{
  GList *item;

  for (item = list; item; item = g_list_next(item))
    {
      _append_item_to_list_store(window, (ab_contact_t *)item->data);
    }
}

static void
tribe_window_finalize(GObject *object)
{
  TribeWindow *window = TRIBE_WINDOW(object);

  G_OBJECT_CLASS(tribe_window_parent_class)->finalize(object);
}

static void
tribe_window_class_init(TribeWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = tribe_window_finalize;

  g_type_class_add_private(klass, sizeof(TribeWindowPrivate));
}

static void
tribe_window_init(TribeWindow *window)
{
  window->priv = G_TYPE_INSTANCE_GET_PRIVATE(window, TRIBE_TYPE_WINDOW, TribeWindowPrivate);
}

GtkWidget *
tribe_window_new(TribeApplication *application)
{
  TribeWindow *window;
  TribeWindowPrivate *priv;
  GtkWidget *vbox;
  GtkWidget *menubar;
  GtkWidget *menuitem;
  GtkWidget *scrolledwin;
  GList *list;

  /*
   * Main window
   */
  window = g_object_new(TRIBE_TYPE_WINDOW, NULL);
  priv = window->priv;
  gtk_window_set_title(GTK_WINDOW(window), APPTITLE);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window),
			      INIT_WINDOW_WIDTH,
			      INIT_WINDOW_HEIGHT);

  g_signal_connect(G_OBJECT(window), "size-allocate",
                   G_CALLBACK(_on_size_allocate_cb), NULL);

    /*
   * Vertical box
   */
  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  /*
   * Menubar
   */
  menubar = _create_menubar(window);
  gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, TRUE, 0);

  /*
   * Toolbar
   */
  _create_toolbar(window);
  gtk_box_pack_start(GTK_BOX(vbox), priv->toolbar, FALSE, TRUE, 0);

  /*
   * Scrolled window
   */
  scrolledwin = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(vbox), scrolledwin, TRUE, TRUE, 0);

  /*
   * List view
   */
  _create_list_view(window);
  gtk_container_add(GTK_CONTAINER(scrolledwin), priv->list_view);

  /*
   * Statusbar
   */
  priv->statusbar = gtk_statusbar_new();
  gtk_box_pack_start(GTK_BOX(vbox), priv->statusbar, FALSE, TRUE, 0);

  priv->statusbar_cid = gtk_statusbar_get_context_id(GTK_STATUSBAR(priv->statusbar),
						     "contact-message");

  /*
   * Set up menu items
   */
  menuitem = gtk_ui_manager_get_widget(priv->ui_manager, "/MainMenu/ViewMenu/ToolBar");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), TRUE);
  menuitem = gtk_ui_manager_get_widget(priv->ui_manager, "/MainMenu/ViewMenu/StatusBar");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), TRUE);

  menuitem = gtk_ui_manager_get_widget(priv->ui_manager, "/MainMenu/ViewMenu/ToolBarStyleMenu/TextBelowIcon");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), TRUE);

  gtk_widget_grab_focus(priv->list_view);

  ab_enum_contacts(&list);

  _populate_list_view(window, list);

  return GTK_WIDGET(window);
}
