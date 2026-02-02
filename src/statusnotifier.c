#include <string.h>

#include "statusnotifier.h"

static gchar* sn_item_get_string_property (const gchar *bus_name, const gchar *obj_path, const gchar *prop_name) {
  GError *error;
  GVariant *result;
  gchar *value;

  error = NULL;
  value = NULL;
  result = g_dbus_connection_call_sync (
    g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL),
    bus_name, obj_path, "org.freedesktop.DBus.Properties", "Get",
    g_variant_new ("(ss)", "org.kde.StatusNotifierItem", prop_name),
    G_VARIANT_TYPE ("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error
  );

  if (result != NULL) {
    GVariant *inner;
    g_variant_get (result, "(v)", &inner);
    value = g_strdup (g_variant_get_string (inner, NULL));
    g_variant_unref (inner);
    g_variant_unref (result);
  }
  else {
    g_error_free (error);
  }

  return value;
}

void sn_item_free (gpointer data) {
  SnItem *item = (SnItem*) data;
  g_free (item->id);
  g_free (item->title);
  g_free (item->icon_name);
  g_free (item);
}

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

GList* watcher_sn_items (GDBusProxy* watcher) {
  GVariant* prop;
  GVariantIter iter;
  GList* list;

  prop = g_dbus_proxy_get_cached_property (watcher, "RegisteredStatusNotifierItems");
  list = NULL;

  if (prop == NULL) return NULL;

  g_variant_iter_init (&iter, prop);

  const gchar* service;
  while (g_variant_iter_next (&iter, "&s", &service)) {
    const gchar *bus_name;
    const gchar *obj_path;
    gchar *slash;

    slash = strchr (service, '/');

    if (slash != NULL) {
      bus_name = g_strndup (service, slash - service);
      obj_path = slash;
    }
    else {
      bus_name = service;
      obj_path = "/StatusNotifierItem";
    }

    SnItem *item = g_new0 (SnItem, 1);

    item->id = sn_item_get_string_property (bus_name, obj_path, "Id");
    item->title = sn_item_get_string_property (bus_name, obj_path, "Title");
    item->icon_name = sn_item_get_string_property (bus_name, obj_path, "IconName");

    if (slash != NULL) {
      g_free ((gchar*) bus_name);
    }

    list = g_list_prepend (list, item);
  }

  g_variant_unref (prop);

  return g_list_reverse (list);
}

void on_properties_changed (GDBusProxy *proxy, GVariant *changed, GStrv invalidated, HiddenApps* instance) {
  instance->sn_items = watcher_sn_items(instance->watcher);
}

void on_item_registered(GDBusProxy *proxy, gchar *sender, gchar *signal, GVariant *params, HiddenApps* instance) {
  instance->sn_items = watcher_sn_items(instance->watcher);
}
