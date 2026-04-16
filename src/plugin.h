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
  gboolean menu_active;

  GtkWidget* settings_dialog;
} HiddenApps;

#endif
