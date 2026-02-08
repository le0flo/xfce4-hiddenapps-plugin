#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <glib.h>
#include <libxfce4panel/libxfce4panel.h>

#define DEFAULT_CONFIG_MAX_COLUMNS 5

typedef struct {
  gint max_columns;
} Config;

void config_save (XfcePanelPlugin* plugin, Config* instance);
void config_read (XfcePanelPlugin* plugin, Config* instance);

#endif
