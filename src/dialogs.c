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

#include "dialogs.h"

static void configure_response (GtkWidget* dialog, gint response, Plugin* instance) {
  gboolean result;

  if (response == GTK_RESPONSE_HELP) {
    result = g_spawn_command_line_async ("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL);

    if (G_UNLIKELY (result == FALSE)) {
      g_warning (_ ("Unable to open the following url: %s"), PLUGIN_WEBSITE);
    }
  }
  else {
    g_object_set_data (G_OBJECT (instance->plugin), "dialog", NULL);

    settings_save (instance->plugin, instance);

    gtk_widget_destroy (dialog);
  }
}

void dialog_configure (XfcePanelPlugin *plugin, Plugin *instance) {
  GtkWidget *dialog;

  if (instance->settings_dialog != NULL) {
    gtk_window_present (GTK_WINDOW (instance->settings_dialog));
    return;
  }

  /* create the dialog */
  instance->settings_dialog = dialog = xfce_titled_dialog_new_with_mixed_buttons (
    _ ("Hidden apps"),
    GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
    GTK_DIALOG_DESTROY_WITH_PARENT, "help-browser-symbolic", _ ("_Help"),
    GTK_RESPONSE_HELP, "window-close-symbolic", _ ("_Close"), GTK_RESPONSE_OK,
    NULL
  );

  g_object_add_weak_pointer (G_OBJECT (instance->settings_dialog), (gpointer*) &instance->settings_dialog);

  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-settings");

  g_object_set_data (G_OBJECT (plugin), "dialog", dialog);
  g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK (configure_response), instance);

  gtk_widget_show (dialog);
}

void dialog_about (XfcePanelPlugin *plugin) {
  const gchar *auth[] = {"Leonardo <noreply@leoflo.me>", "Xfce development team <xfce4-dev@xfce.org>", NULL};

  gtk_show_about_dialog(
    NULL, "logo-icon-name", "xfce4-hiddenapps-plugin",
    "license", xfce_get_license_text (XFCE_LICENSE_TEXT_GPL),
    "version", VERSION_FULL,
    "program-name", PACKAGE_NAME,
    "comments", _("Windows like system tray plugin"),
    "website", PLUGIN_WEBSITE,
    "copyright", "Copyright \xc2\xa9 2006-" COPYRIGHT_YEAR " Leonardo",
    "authors", auth,
    NULL
  );
}
