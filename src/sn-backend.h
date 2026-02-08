#ifndef __SN_BACKEND_H__
#define __SN_BACKEND_H__

#include <gio/gio.h>

#define SN_WATCHER_BUS_NAME "org.freedesktop.StatusNotifierWatcher"
#define SN_WATCHER_OBJ_PATH "/StatusNotifierWatcher"

#define SN_WATCHER_INTROSPECTION_PATH "/org/freedesktop/StatusNotifierWatcher/org.freedesktop.StatusNotifierWatcher.xml"

typedef struct {
  GDBusConnection* connection;
  guint bus_name_id;
  guint registration_id;
  GList* registered_items;   /* list of gchar* service names */
  gboolean host_registered;
} SnBackend;

SnBackend* sn_backend_new (void);

void sn_backend_init (SnBackend* backend);
void sn_backend_deinit (SnBackend* backend);

#endif
