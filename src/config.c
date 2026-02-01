#include "config.h"

void settings_save (XfcePanelPlugin* plugin, Config* instance) {
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

void settings_read (XfcePanelPlugin* plugin, Config* instance) {
  XfceRc* rc;
  gchar* file;

  file = xfce_panel_plugin_save_location (plugin, TRUE);

  if (G_LIKELY (file != NULL)) {
    rc = xfce_rc_simple_open (file, TRUE);
    g_free (file);

    if (G_LIKELY (rc != NULL)) {
      instance->max_columns = xfce_rc_read_int_entry (rc, "max_columns", DEFAULT_CONFIG_MAX_COLUMNS);
      xfce_rc_close (rc);

      return;
    }
  }

  DBG ("Applying default settings");
  instance->max_columns = DEFAULT_CONFIG_MAX_COLUMNS;
}
