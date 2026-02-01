#include "statusnotifier.h"

GDBusProxy* watcher_new (void) {
  GError* error;
  GDBusProxy* watcher;

  error = NULL;
  watcher = g_dbus_proxy_new_for_bus_sync(
    G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL,
    "org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher",
    "org.kde.StatusNotifierWatcher", NULL, &error
  );

  if (watcher == NULL) {
    g_printerr("Failed to connect to StatusNotifierWatcher: %s\n", error->message);
    g_error_free(error);
    return NULL;
  }

  return watcher;
}

void watcher_sn_items (GDBusProxy* watcher) {
  // TODO: da fare
}

void on_properties_changed (GDBusProxy *proxy, GVariant *changed, GStrv invalidated, HiddenApps* instance) {
  // TODO: da fare
}

void on_item_registered(GDBusProxy *proxy, gchar *sender, gchar *signal, GVariant *params, HiddenApps* instance) {
  // TODO: da fare
}
