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
