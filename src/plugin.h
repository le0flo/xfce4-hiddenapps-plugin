#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "sn-backend.h"
#include "config.h"

typedef struct {
  XfcePanelPlugin* plugin;
  SnBackend* backend;
  Config* config;

  GtkWidget* item_ebox;
  GtkWidget* item_hvbox;
  GtkWidget* item_button;

  GtkWidget* menu;
  GtkWidget* settings_dialog;
} HiddenApps;

#endif
