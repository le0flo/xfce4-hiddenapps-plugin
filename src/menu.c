#include "statusnotifier.h"
#include "menu.h"

void menu_build (XfcePanelPlugin* plugin, HiddenApps* instance) {
  instance->watcher = watcher_new ();
  instance->menu = gtk_menu_new ();

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
