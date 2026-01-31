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

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <string.h>
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>

#define DEFAULT_MAX_COLUMNS 5

G_BEGIN_DECLS

typedef struct {
  XfcePanelPlugin *plugin;

  /* panel widgets */
  GtkWidget* ebox;
  GtkWidget* hvbox;
  GtkWidget* label;

  /* sample settings */
  GtkWidget* settings_dialog;
  gint max_columns;
} Plugin;

void settings_save(XfcePanelPlugin* plugin, Plugin* instance);

G_END_DECLS

#endif
