#include "sn-item.h"

static gchar* get_string_prop (GDBusProxy* proxy, const gchar* prop) {
  GVariant* v = g_dbus_proxy_get_cached_property (proxy, prop);

  if (v == NULL) {
    return NULL;
  }

  gchar* s = g_variant_dup_string (v, NULL);
  g_variant_unref (v);

  return s;
}

static guint32 get_uint32_prop (GDBusProxy* proxy, const gchar* prop) {
  GVariant* v = g_dbus_proxy_get_cached_property (proxy, prop);

  if (v == NULL) {
    return 0;
  }

  guint32 val = g_variant_get_uint32 (v);
  g_variant_unref (v);

  return val;
}

static gboolean get_boolean_prop (GDBusProxy* proxy, const gchar* prop) {
  GVariant* v = g_dbus_proxy_get_cached_property (proxy, prop);

  if (v == NULL) {
    return FALSE;
  }

  gboolean val = g_variant_get_boolean (v);
  g_variant_unref (v);

  return val;
}

static gchar* get_object_path_prop (GDBusProxy* proxy, const gchar* prop) {
  GVariant* v = g_dbus_proxy_get_cached_property (proxy, prop);

  if (v == NULL) {
    return NULL;
  }

  gchar* s = g_strdup (g_variant_get_string (v, NULL));
  g_variant_unref (v);

  return s;
}

static void cache_tooltip (SnItem* item) {
  GVariant* v = g_dbus_proxy_get_cached_property (item->proxy, "ToolTip");

  if (v == NULL) {
    return;
  }

  const gchar* icon_name = NULL;
  const gchar* title = NULL;
  const gchar* body = NULL;
  GVariant* pixmap_array = NULL;

  g_variant_get (v, "(&s@a(iiay)&s&s)", &icon_name, &pixmap_array, &title, &body);

  g_free (item->tooltip_icon_name);
  g_free (item->tooltip_title);
  g_free (item->tooltip_body);

  item->tooltip_icon_name = g_strdup (icon_name);
  item->tooltip_title = g_strdup (title);
  item->tooltip_body = g_strdup (body);

  if (pixmap_array) {
    g_variant_unref (pixmap_array);
  }

  g_variant_unref (v);
}

static void refresh_properties (SnItem* item) {
  GDBusProxy* p = item->proxy;

  g_free (item->category);
  g_free (item->id);
  g_free (item->title);
  g_free (item->status);
  g_free (item->icon_name);
  g_free (item->overlay_icon_name);
  g_free (item->attention_icon_name);
  g_free (item->attention_movie_name);
  g_free (item->menu_path);

  item->category = get_string_prop (p, "Category");
  item->id = get_string_prop (p, "Id");
  item->title = get_string_prop (p, "Title");
  item->status = get_string_prop (p, "Status");
  item->window_id = get_uint32_prop (p, "WindowId");
  item->icon_name = get_string_prop (p, "IconName");
  item->overlay_icon_name = get_string_prop (p, "OverlayIconName");
  item->attention_icon_name = get_string_prop (p, "AttentionIconName");
  item->attention_movie_name = get_string_prop (p, "AttentionMovieName");
  item->item_is_menu = get_boolean_prop (p, "ItemIsMenu");
  item->menu_path = get_object_path_prop (p, "Menu");

  cache_tooltip (item);
}

static void on_properties_changed (
  GDBusProxy* proxy,
  GVariant* changed_properties,
  GStrv invalidated_properties,
  gpointer user_data
) {
  SnItem* item = user_data;
  refresh_properties (item);
}

static void on_signal (
  GDBusProxy*  proxy,
  const gchar* sender_name,
  const gchar* signal_name,
  GVariant*    parameters,
  gpointer     user_data
) {
  SnItem* item = user_data;

  if (g_strcmp0 (signal_name, "NewTitle") == 0) {
    g_free (item->title);
    item->title = get_string_prop (proxy, "Title");
  }
  else if (g_strcmp0 (signal_name, "NewIcon") == 0) {
    g_free (item->icon_name);
    item->icon_name = get_string_prop (proxy, "IconName");
  }
  else if (g_strcmp0 (signal_name, "NewOverlayIcon") == 0) {
    g_free (item->overlay_icon_name);
    item->overlay_icon_name = get_string_prop (proxy, "OverlayIconName");
  }
  else if (g_strcmp0 (signal_name, "NewAttentionIcon") == 0) {
    g_free (item->attention_icon_name);
    item->attention_icon_name = get_string_prop (proxy, "AttentionIconName");
  }
  else if (g_strcmp0 (signal_name, "NewToolTip") == 0) {
    cache_tooltip (item);
  }
  else if (g_strcmp0 (signal_name, "NewStatus") == 0) {
    const gchar* new_status = NULL;
    g_variant_get (parameters, "(&s)", &new_status);
    g_free (item->status);
    item->status = g_strdup (new_status);
  }
}

static void on_proxy_ready (GObject* source, GAsyncResult* res, gpointer user_data) {
  SnItem* item = user_data;
  GError* error = NULL;

  item->proxy = g_dbus_proxy_new_finish (res, &error);

  if (item->proxy == NULL) {
    g_warning ("Failed to create proxy for %s %s: %s", item->bus_name, item->obj_path, error->message);
    g_error_free (error);
    return;
  }

  g_signal_connect (item->proxy, "g-properties-changed", G_CALLBACK (on_properties_changed), item);
  g_signal_connect (item->proxy, "g-signal", G_CALLBACK (on_signal), item);

  refresh_properties (item);

  g_debug (
    "SnItem proxy ready for %s %s (id=%s, title=%s)",
    item->bus_name, item->obj_path,
    item->id ? item->id : "(null)",
    item->title ? item->title : "(null)"
  );
}

SnItem* sn_item_get (GDBusConnection* connection, const gchar* bus_name, const gchar* obj_path) {
  g_return_val_if_fail (connection != NULL, NULL);
  g_return_val_if_fail (bus_name != NULL, NULL);

  SnItem* item = g_slice_new0 (SnItem);

  item->connection = g_object_ref (connection);
  item->bus_name   = g_strdup (bus_name);
  item->obj_path   = g_strdup (obj_path ? obj_path : "/StatusNotifierItem");

  GBytes* xml_bytes = g_resources_lookup_data (
    SN_ITEM_INTROSPECTION_PATH,
    G_RESOURCE_LOOKUP_FLAGS_NONE,
    NULL
  );

  GDBusInterfaceInfo* iface_info = NULL;
  if (xml_bytes != NULL) {
    const gchar* xml_data = g_bytes_get_data (xml_bytes, NULL);
    GDBusNodeInfo* node_info = g_dbus_node_info_new_for_xml (xml_data, NULL);
    g_bytes_unref (xml_bytes);

    if (node_info != NULL) {
      iface_info = g_dbus_node_info_lookup_interface (node_info, SN_ITEM_INTERFACE);

      if (iface_info != NULL) {
        iface_info = g_dbus_interface_info_ref (iface_info);
      }

      g_dbus_node_info_unref (node_info);
    }
  }

  g_dbus_proxy_new (
    connection,
    G_DBUS_PROXY_FLAGS_NONE,
    iface_info,
    item->bus_name,
    item->obj_path,
    SN_ITEM_INTERFACE,
    NULL,
    on_proxy_ready,
    item
  );

  if (iface_info != NULL) {
    g_dbus_interface_info_unref (iface_info);
  }

  return item;
}

void sn_item_free (SnItem* item) {
  if (item == NULL) {
    return;
  }

  if (item->proxy != NULL) {
    g_signal_handlers_disconnect_by_data (item->proxy, item);
    g_object_unref (item->proxy);
  }

  if (item->connection != NULL) {
    g_object_unref (item->connection);
  }

  g_free (item->bus_name);
  g_free (item->obj_path);
  g_free (item->category);
  g_free (item->id);
  g_free (item->title);
  g_free (item->status);
  g_free (item->icon_name);
  g_free (item->overlay_icon_name);
  g_free (item->attention_icon_name);
  g_free (item->attention_movie_name);
  g_free (item->tooltip_icon_name);
  g_free (item->tooltip_title);
  g_free (item->tooltip_body);
  g_free (item->menu_path);

  g_slice_free (SnItem, item);
}

static void call_method_xy (SnItem* item, const gchar* method, gint x, gint y) {
  if (item->proxy == NULL) {
    return;
  }

  g_dbus_proxy_call (
    item->proxy,
    method,
    g_variant_new ("(ii)", x, y),
    G_DBUS_CALL_FLAGS_NONE,
    -1, NULL, NULL, NULL
  );
}

void sn_item_activate (SnItem* item, gint x, gint y) {
  call_method_xy (item, "Activate", x, y);
}

void sn_item_secondary_activate (SnItem* item, gint x, gint y) {
  call_method_xy (item, "SecondaryActivate", x, y);
}

void sn_item_context_menu (SnItem* item, gint x, gint y) {
  call_method_xy (item, "ContextMenu", x, y);
}

void sn_item_scroll (SnItem* item, gint delta, const gchar* orientation) {
  if (item->proxy == NULL) {
    return;
  }

  g_dbus_proxy_call (
    item->proxy,
    "Scroll",
    g_variant_new ("(is)", delta, orientation),
    G_DBUS_CALL_FLAGS_NONE,
    -1, NULL, NULL, NULL
  );
}
