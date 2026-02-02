#include "menu.h"
#include "gtk/gtk.h"
#include "statusnotifier.h"

void menu_build (XfcePanelPlugin* plugin, HiddenApps* instance) {
  instance->watcher = watcher_new ();
  instance->menu = gtk_menu_new ();
  instance->sn_items = watcher_sn_items (instance->watcher);

  g_print ("sn_items count: %d\n", g_list_length (instance->sn_items));

  GtkWidget *grid = gtk_grid_new ();
  gint col = 0, row = 0;

  for (GList *l = instance->sn_items; l != NULL; l = l->next) {
    SnItem *sn = (SnItem*) l->data;
    g_print ("item: id=%s title=%s icon=%s\n", sn->id, sn->title, sn->icon_name);
    GtkWidget *image = gtk_image_new_from_icon_name (sn->icon_name, GTK_ICON_SIZE_BUTTON);
    gtk_grid_attach (GTK_GRID (grid), image, col, row, 1, 1);

    col++;
    if (col >= instance->config->max_columns) {
      col = 0;
      row++;
    }
  }

  gtk_widget_show_all (grid);

  GtkWidget *menu_item = gtk_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (menu_item), grid);
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (instance->menu), menu_item);

  g_signal_connect(instance->watcher, "g-properties-changed", G_CALLBACK(on_properties_changed), instance);
  g_signal_connect(instance->watcher, "g-signal", G_CALLBACK(on_item_registered), instance);
}

static void menu_deactivate (GtkMenuShell* menu, gpointer user_data) {
  GtkToggleButton *button = GTK_TOGGLE_BUTTON (user_data);

  gtk_toggle_button_set_active (button, FALSE);
}

gboolean menu_show(GtkWidget *widget, GdkEventButton* event, HiddenApps* instance) {
  GtkMenu *menu = GTK_MENU (instance->menu);

  if (event->button == 1) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (instance->item_button), TRUE);
    g_signal_handlers_disconnect_by_func (menu, menu_deactivate, instance->item_button);
    g_signal_connect (menu, "deactivate", G_CALLBACK (menu_deactivate), instance->item_button);
    gtk_menu_popup_at_widget (menu, widget, GDK_GRAVITY_SOUTH, GDK_GRAVITY_NORTH, (GdkEvent*) event);

    return TRUE;
  }

  return FALSE;
}
