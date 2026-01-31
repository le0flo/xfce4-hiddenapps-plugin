/*  $Id$
 *
 *  Copyright (C) 2025 Leonardo <noreply@leoflo.me>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "plugin.h"
#include "dialogs.h"

void settings_save (XfcePanelPlugin* plugin, Plugin* instance) {
  XfceRc* rc;
  gchar* file;

  file = xfce_panel_plugin_save_location (plugin, TRUE);

  if (G_UNLIKELY (file == NULL)) {
    DBG ("Failed to open config file");
    return;
  }

  rc = xfce_rc_simple_open (file, FALSE);
  g_free (file);

  if (G_LIKELY (rc != NULL)) {
    xfce_rc_write_int_entry (rc, "max_columns", instance->max_columns);
    xfce_rc_close (rc);
  }
}

static void settings_read (Plugin* instance) {
  XfceRc* rc;
  gchar* file;

  file = xfce_panel_plugin_save_location (instance->plugin, TRUE);

  if (G_LIKELY (file != NULL)) {
    rc = xfce_rc_simple_open (file, TRUE);
    g_free (file);

    if (G_LIKELY (rc != NULL)) {
      instance->max_columns = xfce_rc_read_int_entry (rc, "max_columns", DEFAULT_MAX_COLUMNS);
      xfce_rc_close (rc);

      return;
    }
  }

  DBG ("Applying default settings");
  instance->max_columns = DEFAULT_MAX_COLUMNS;
}

static Plugin* plugin_new (XfcePanelPlugin* plugin) {
  Plugin* instance;
  GtkOrientation orientation;
  GtkWidget* label;

  instance = g_slice_new0 (Plugin);
  instance->plugin = plugin;

  settings_read (instance);
  orientation = xfce_panel_plugin_get_orientation (plugin);

  instance->ebox = gtk_event_box_new ();
  gtk_widget_show (instance->ebox);

  instance->hvbox = gtk_box_new (orientation, 2);
  gtk_widget_show (instance->hvbox);
  gtk_container_add (GTK_CONTAINER (instance->ebox), instance->hvbox);

  label = gtk_label_new (_ ("^"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (instance->hvbox), label, FALSE, FALSE, 0);

  return instance;
}

static void plugin_free (XfcePanelPlugin* plugin, Plugin* instance) {
  GtkWidget* dialog;

  dialog = g_object_get_data (G_OBJECT (plugin), "dialog");
  if (G_UNLIKELY (dialog != NULL)) gtk_widget_destroy (dialog);

  gtk_widget_destroy (instance->hvbox);

  g_slice_free (Plugin, instance);
}

static void plugin_orientation_changed (XfcePanelPlugin* plugin, GtkOrientation orientation, Plugin* instance) {
  gtk_orientable_set_orientation (GTK_ORIENTABLE (instance->hvbox), orientation);
}

static gboolean plugin_size_changed (XfcePanelPlugin* plugin, gint size, Plugin* instance) {
  GtkOrientation orientation;

  orientation = xfce_panel_plugin_get_orientation (plugin);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, size);
  }
  else {
    gtk_widget_set_size_request (GTK_WIDGET (plugin), size, -1);
  }

  return TRUE;
}

static void plugin_construct (XfcePanelPlugin *plugin) {
  Plugin* instance;

  /* setup transation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* create the plugin */
  instance = plugin_new (plugin);

  /* add the ebox to the panel */
  gtk_container_add (GTK_CONTAINER (plugin), instance->ebox);

  /* show the panel's right-click menu on this ebox */
  xfce_panel_plugin_add_action_widget (plugin, instance->ebox);

  /* connect plugin signals */
  g_signal_connect (G_OBJECT (plugin), "free-data", G_CALLBACK (plugin_free), instance);
  g_signal_connect (G_OBJECT (plugin), "save", G_CALLBACK (settings_save), instance);
  g_signal_connect (G_OBJECT (plugin), "orientation-changed", G_CALLBACK (plugin_orientation_changed), instance);
  g_signal_connect (G_OBJECT (plugin), "size-changed", G_CALLBACK (plugin_size_changed), instance);

  /* show the configure menu item and connect signal */
  xfce_panel_plugin_menu_show_configure (plugin);
  g_signal_connect (G_OBJECT (plugin), "configure-plugin", G_CALLBACK (dialog_configure), instance);

  /* show the about menu item and connect signal */
  xfce_panel_plugin_menu_show_about (plugin);
  g_signal_connect (G_OBJECT (plugin), "about", G_CALLBACK (dialog_about), NULL);
}

XFCE_PANEL_PLUGIN_REGISTER (plugin_construct);
