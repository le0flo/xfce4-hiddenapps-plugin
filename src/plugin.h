#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "config.h"

G_BEGIN_DECLS

typedef struct {
  XfcePanelPlugin* plugin;
  Config* config;

  GtkWidget* item_ebox;
  GtkWidget* item_hvbox;
  GtkWidget* item_button;

  GtkWidget* menu;
  GDBusProxy* watcher;
  GList* sn_items;

  GtkWidget* settings_dialog;
} HiddenApps;

G_END_DECLS

#endif
