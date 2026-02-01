#include "statusnotifier.h"
#include "menu.h"

void menu_build (XfcePanelPlugin* plugin, HiddenApps* instance) {
  instance->watcher = watcher_new ();
  instance->menu = gtk_menu_new ();

  g_signal_connect(instance->watcher, "g-properties-changed", G_CALLBACK(on_properties_changed), instance);
  g_signal_connect(instance->watcher, "g-signal", G_CALLBACK(on_item_registered), instance);
}

gboolean menu_show(GtkWidget *widget, GdkEventButton* event, gpointer user_data) {
  GtkMenu *menu = GTK_MENU(user_data);

  if (event->button == 1) {
    gtk_menu_popup_at_widget(menu, widget, GDK_GRAVITY_SOUTH_WEST, GDK_GRAVITY_NORTH_WEST, (GdkEvent*) event);

    return TRUE;
  }

  return FALSE;
}
