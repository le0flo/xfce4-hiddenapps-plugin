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

#ifndef __SN_BACKEND_H__
#define __SN_BACKEND_H__

#include <gio/gio.h>

#define SN_WATCHER_BUS_NAME "org.freedesktop.StatusNotifierWatcher"
#define SN_WATCHER_KDE_BUS_NAME "org.kde.StatusNotifierWatcher"

#define SN_WATCHER_OBJ_PATH "/StatusNotifierWatcher"

#define SN_WATCHER_INTERFACE "org.freedesktop.StatusNotifierWatcher"
#define SN_WATCHER_KDE_INTERFACE "org.kde.StatusNotifierWatcher"

#define SN_WATCHER_INTROSPECTION_PATH "/org/freedesktop/StatusNotifierWatcher/org.freedesktop.StatusNotifierWatcher.xml"
#define SN_WATCHER_KDE_INTROSPECTION_PATH "/org/kde/StatusNotifierWatcher/org.kde.StatusNotifierWatcher.xml"

typedef struct _SnBackend SnBackend;

typedef void (*SnBackendChangedCallback) (SnBackend* backend, gpointer user_data);

struct _SnBackend {
  GDBusConnection* connection;

  guint bus_name_id;
  guint kde_bus_name_id;
  guint registration_id;
  guint kde_registration_id;

  GList* items;

  SnBackendChangedCallback changed_callback;
  gpointer changed_callback_data;
};

SnBackend* sn_backend_new (void);

void sn_backend_init (SnBackend* backend);
void sn_backend_deinit (SnBackend* backend);

void sn_backend_set_changed_callback (SnBackend* backend, SnBackendChangedCallback cb, gpointer user_data);

#endif
