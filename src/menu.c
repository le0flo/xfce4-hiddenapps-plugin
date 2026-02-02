#include "menu.h"
#include "gtk/gtk.h"
#include "statusnotifier.h"

void menu_refresh (HiddenApps* instance) {
  GList *children, *iter;

  if (instance->sn_items != NULL) {
    g_list_free_full (instance->sn_items, sn_item_free);
  }
  instance->sn_items = watcher_sn_items (instance->watcher);

  children = gtk_container_get_children (GTK_CONTAINER (instance->menu));
  for (iter = children; iter != NULL; iter = iter->next) {
    gtk_widget_destroy (GTK_WIDGET (iter->data));
  }
  g_list_free (children);

  if (instance->sn_items == NULL)
    return;

  for (GList *l = instance->sn_items; l != NULL; l = l->next) {
    SnItem *sn = (SnItem*) l->data;

    GtkWidget *image = gtk_image_new_from_icon_name (sn->icon_name, GTK_ICON_SIZE_LARGE_TOOLBAR);
    GtkWidget *label = gtk_label_new (sn->title ? sn->title : "");

    GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

    GtkWidget *menu_item = gtk_menu_item_new ();
    gtk_container_add (GTK_CONTAINER (menu_item), box);
    gtk_widget_show_all (menu_item);
    gtk_menu_shell_append (GTK_MENU_SHELL (instance->menu), menu_item);
  }
}

void menu_build (XfcePanelPlugin* plugin, HiddenApps* instance) {
  instance->watcher = watcher_new ();
  instance->menu = gtk_menu_new ();
  instance->sn_items = NULL;

  menu_refresh (instance);

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
