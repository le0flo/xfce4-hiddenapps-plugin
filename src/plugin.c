/*
Copyright (C) 2026 leoflo

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "plugin.h"

#include "menu.h"
#include "dialogs.h"

static void on_orientation_changed (XfcePanelPlugin* plugin, GtkOrientation orientation, HiddenApps* instance);
static void on_size_changed (XfcePanelPlugin* plugin, gint size, HiddenApps* instance);
static void on_backend_changed (SnBackend* backend, gpointer user_data);

static HiddenApps* hiddenapps_new (void) {
  HiddenApps* instance = g_slice_new0 (HiddenApps);
  return instance;
}

static void hiddenapps_init (HiddenApps* instance, XfcePanelPlugin* plugin) {
  instance->plugin = plugin;
  instance->backend = sn_backend_new();
  instance->config = config_new ();

  GtkOrientation orientation = xfce_panel_plugin_get_orientation (plugin);

  sn_backend_init (instance->backend);
  sn_backend_set_changed_callback (instance->backend, on_backend_changed, instance);

  config_read (instance->config, instance->plugin);
  menu_build (instance);

  instance->item_ebox = gtk_event_box_new ();
  gtk_widget_show (instance->item_ebox);

  instance->item_hvbox = gtk_box_new (orientation, 2);
  gtk_widget_show (instance->item_hvbox);
  gtk_container_add (GTK_CONTAINER (instance->item_ebox), instance->item_hvbox);

  instance->item_button = gtk_toggle_button_new ();
  gtk_button_set_image (GTK_BUTTON (instance->item_button), gtk_image_new_from_icon_name ("adw-expander-arrow-symbolic", GTK_ICON_SIZE_BUTTON));
  gtk_button_set_relief (GTK_BUTTON (instance->item_button), GTK_RELIEF_NONE);
  gtk_widget_show (instance->item_button);
  gtk_box_pack_start (GTK_BOX (instance->item_hvbox), instance->item_button, FALSE, TRUE, 0);

  g_signal_connect (instance->item_button, "button-press-event", G_CALLBACK (menu_show), instance);
}

static void hiddenapps_free (XfcePanelPlugin* plugin, HiddenApps* instance) {
  GtkWidget* dialog = g_object_get_data (G_OBJECT (plugin), "dialog");

  if (G_UNLIKELY (dialog != NULL)) {
    gtk_widget_destroy (dialog);
  }

  gtk_widget_destroy (instance->item_button);
  gtk_widget_destroy (instance->item_hvbox);

  config_save (instance->config, instance->plugin);
  sn_backend_deinit (instance->backend);

  g_slice_free (HiddenApps, instance);
}

static void hiddenapps_register (XfcePanelPlugin *plugin) {
  HiddenApps* instance = hiddenapps_new ();
  hiddenapps_init(instance, plugin);

  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  gtk_container_add (GTK_CONTAINER (plugin), instance->item_ebox);
  xfce_panel_plugin_add_action_widget (plugin, instance->item_ebox);

  g_signal_connect (G_OBJECT (plugin), "free-data", G_CALLBACK (hiddenapps_free), instance);
  g_signal_connect (G_OBJECT (plugin), "save", G_CALLBACK (config_save), instance);
  g_signal_connect (G_OBJECT (plugin), "orientation-changed", G_CALLBACK (on_orientation_changed), instance);
  g_signal_connect (G_OBJECT (plugin), "size-changed", G_CALLBACK (on_size_changed), instance);

  xfce_panel_plugin_menu_show_configure (plugin);
  g_signal_connect (G_OBJECT (plugin), "configure-plugin", G_CALLBACK (dialog_configure), instance);

  xfce_panel_plugin_menu_show_about (plugin);
  g_signal_connect (G_OBJECT (plugin), "about", G_CALLBACK (dialog_about), NULL);
}

XFCE_PANEL_PLUGIN_REGISTER (hiddenapps_register);

static void on_orientation_changed (XfcePanelPlugin* plugin, GtkOrientation orientation, HiddenApps* instance) {
  gtk_orientable_set_orientation (GTK_ORIENTABLE (instance->item_hvbox), orientation);
}

static void on_size_changed (XfcePanelPlugin* plugin, gint size, HiddenApps* instance) {
  GtkOrientation orientation = xfce_panel_plugin_get_orientation (plugin);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, size);
  }
  else {
    gtk_widget_set_size_request (GTK_WIDGET (plugin), size, -1);
  }
}

static void on_backend_changed (SnBackend* backend, gpointer user_data) {
  HiddenApps* instance = user_data;
  menu_refresh (instance);
}
