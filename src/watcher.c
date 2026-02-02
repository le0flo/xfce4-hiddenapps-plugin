#include "statusnotifier.h"

GDBusProxy* watcher_new (void) {
  GError* error = NULL;
  GDBusProxy* watcher = g_dbus_proxy_new_for_bus_sync (
    G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL,
    "org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher",
    "org.kde.StatusNotifierWatcher", NULL, &error
  );

  if (watcher == NULL) {
    g_printerr ("Failed to connect to StatusNotifierWatcher: %s\n", error->message);
    g_error_free (error);

    return NULL;
  }

  return watcher;
}

GList* watcher_sn_items (GDBusProxy* watcher) {
  GVariant* prop = g_dbus_proxy_get_cached_property (watcher, "RegisteredStatusNotifierItems");

  if (prop == NULL) return NULL;

  GList* list = NULL;
  GVariantIter iter;
  g_variant_iter_init (&iter, prop);

  const gchar* service;
  while (g_variant_iter_next (&iter, "&s", &service)) {
    const gchar* bus_name;
    const gchar* obj_path;

    gchar* slash = strchr (service, '/');

    if (slash != NULL) {
      bus_name = g_strndup (service, slash - service);
      obj_path = slash;
    }
    else {
      bus_name = service;
      obj_path = "/StatusNotifierItem";
    }

    SnItem* item = g_new0 (SnItem, 1);

    item->id = sn_prop_parse_string (sn_item_get_property (bus_name, obj_path, "Id"));
    item->title = sn_prop_parse_string (sn_item_get_property (bus_name, obj_path, "Title"));
    item->icon_name = sn_prop_parse_string (sn_item_get_property (bus_name, obj_path, "IconName"));

    GVariant* pixmap_prop = sn_item_get_property (bus_name, obj_path, "IconPixmap");
    if (pixmap_prop != NULL) {
      item->icon_pixmap = sn_prop_parse_icon_pixmap (pixmap_prop);

      g_variant_unref (pixmap_prop);
    }

    GVariant* tooltip_prop = sn_item_get_property (bus_name, obj_path, "ToolTip");
    if (tooltip_prop != NULL) {
      sn_prop_parse_tooltip (tooltip_prop, &item->tooltip_title, &item->tooltip_description);

      g_variant_unref (tooltip_prop);
    }

    if (slash != NULL) {
      g_free ((gchar*) bus_name);
    }

    list = g_list_prepend (list, item);
  }

  g_variant_unref (prop);

  return g_list_reverse (list);
}
