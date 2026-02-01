#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>

#define DEFAULT_CONFIG_MAX_COLUMNS 5

typedef struct {
  gint max_columns;
} Config;

void settings_save (XfcePanelPlugin* plugin, Config* instance);
void settings_read (XfcePanelPlugin* plugin, Config* instance);

#endif
