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

#ifndef __SN_ITEM_H__
#define __SN_ITEM_H__

#include <gtk/gtk.h>
#include <libdbusmenu-gtk/menu.h>

#define SN_ITEM_INTERFACE_FDO "org.freedesktop.StatusNotifierItem"
#define SN_ITEM_INTERFACE_KDE "org.kde.StatusNotifierItem"

typedef struct _SnItem SnItem;

typedef void (*SnItemChangedCallback) (SnItem* item, gpointer user_data);

struct _SnItem {
  gchar* bus_name;
  gchar* obj_path;
  GDBusProxy* proxy;
  GDBusConnection* connection;

  gchar* category;
  gchar* id;
  gchar* title;
  gchar* status;
  guint32 window_id;

  gchar* icon_name;
  GdkPixbuf* icon_pixmap;

  gchar* overlay_icon_name;

  gchar* attention_icon_name;
  gchar* attention_movie_name;

  gchar* tooltip_icon_name;
  gchar* tooltip_title;
  gchar* tooltip_body;

  gboolean item_is_menu;
  gchar* menu_path;
  DbusmenuGtkMenu* dbus_menu;

  SnItemChangedCallback changed_callback;
  gpointer changed_callback_data;
};

SnItem* sn_item_new (GDBusConnection* connection, const gchar* bus_name, const gchar* obj_path, gboolean use_kde);
void sn_item_free (SnItem* item);

void sn_item_set_changed_callback (SnItem* item, SnItemChangedCallback cb, gpointer user_data);

void sn_item_activate (SnItem* item, gint x, gint y);
void sn_item_secondary_activate (SnItem* item, gint x, gint y);
void sn_item_context_menu (SnItem* item, gint x, gint y);
GtkMenu* sn_item_get_gtk_menu (SnItem* item);
void sn_item_scroll (SnItem* item, gint delta, const gchar* orientation);

#endif
