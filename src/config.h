#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <glib.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4util/libxfce4util.h>

#define DEFAULT_CONFIG_MAX_COLUMNS 5

typedef struct {
  gint max_columns;
} Config;

Config* config_new (void);

void config_save (Config* config, XfcePanelPlugin* plugin);
void config_read (Config* config, XfcePanelPlugin* plugin);

#endif
