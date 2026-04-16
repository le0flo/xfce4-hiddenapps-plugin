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

#include "config.h"

Config* config_new (void) {
  Config* config = g_slice_new(Config);
  return config;
}

void config_save (Config* config, XfcePanelPlugin* plugin) {
  gchar* file = xfce_panel_plugin_save_location (plugin, TRUE);

  if (G_UNLIKELY (file == NULL)) {
    DBG ("Failed to open config file");
    return;
  }

  XfceRc* rc = xfce_rc_simple_open (file, FALSE);

  g_free (file);

  if (G_LIKELY (rc != NULL)) {
    xfce_rc_write_int_entry (rc, "max_columns", config->max_columns);

    xfce_rc_close (rc);
  }
}

void config_read (Config* config, XfcePanelPlugin* plugin) {
  gchar* file = xfce_panel_plugin_save_location (plugin, TRUE);

  if (G_LIKELY (file != NULL)) {
    XfceRc* rc = xfce_rc_simple_open (file, TRUE);

    g_free (file);

    if (G_LIKELY (rc != NULL)) {
      config->max_columns = xfce_rc_read_int_entry (rc, "max_columns", DEFAULT_CONFIG_MAX_COLUMNS);

      xfce_rc_close (rc);

      return;
    }
  }

  DBG ("Applying default settings");
  config->max_columns = DEFAULT_CONFIG_MAX_COLUMNS;
}
