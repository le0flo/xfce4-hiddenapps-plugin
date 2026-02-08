#include "sn-backend.h"

static void handle_method_call (
  GDBusConnection* connection,
  const gchar* sender,
  const gchar* object_path,
  const gchar* interface_name,
  const gchar* method_name,
  GVariant* parameters,
  GDBusMethodInvocation* invocation,
  gpointer user_data
) {
  SnBackend* backend = user_data;

  if (g_strcmp0 (method_name, "RegisterStatusNotifierItem") == 0) {
    const gchar* service = NULL;
    g_variant_get (parameters, "(&s)", &service);

    gchar* item_id;
    if (service[0] == '/') {
      item_id = g_strdup_printf ("%s%s", sender, service);
    }
    else {
      item_id = g_strdup (service);
    }

    gboolean already = FALSE;
    for (GList* l = backend->registered_items; l != NULL; l = l->next) {
      if (g_strcmp0 (l->data, item_id) == 0) {
        already = TRUE;
        break;
      }
    }

    if (!already) {
      backend->registered_items = g_list_append (backend->registered_items, item_id);

      g_dbus_connection_emit_signal (
        connection, NULL,
        SN_WATCHER_OBJ_PATH,
        "org.freedesktop.StatusNotifierWatcher",
        "StatusNotifierItemRegistered",
        g_variant_new ("(s)", item_id),
        NULL
      );

      g_debug ("StatusNotifierItem registered: %s", item_id);
    }
    else {
      g_free (item_id);
    }

    g_dbus_method_invocation_return_value (invocation, NULL);
  }
  else if (g_strcmp0 (method_name, "RegisterStatusNotifierHost") == 0) {
    if (!backend->host_registered) {
      backend->host_registered = TRUE;

      g_dbus_connection_emit_signal (
        connection, NULL,
        SN_WATCHER_OBJ_PATH,
        "org.freedesktop.StatusNotifierWatcher",
        "StatusNotifierHostRegistered",
        NULL, NULL
      );

      g_debug ("StatusNotifierHost registered");
    }

    g_dbus_method_invocation_return_value (invocation, NULL);
  }
  else {
    g_dbus_method_invocation_return_error (
      invocation, G_DBUS_ERROR,
      G_DBUS_ERROR_UNKNOWN_METHOD,
      "Unknown method: %s", method_name
    );
  }
}

static GVariant* handle_get_property (
  GDBusConnection* connection,
  const gchar* sender,
  const gchar* object_path,
  const gchar* interface_name,
  const gchar* property_name,
  GError** error,
  gpointer user_data
) {
  SnBackend* backend = user_data;

  if (g_strcmp0 (property_name, "RegisteredStatusNotifierItems") == 0) {
    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));

    for (GList* l = backend->registered_items; l != NULL; l = l->next) {
      g_variant_builder_add (&builder, "s", (const gchar*) l->data);
    }

    return g_variant_builder_end (&builder);
  }

  else if (g_strcmp0 (property_name, "IsStatusNotifierHostRegistered") == 0) {
    return g_variant_new_boolean (backend->host_registered);
  }

  else if (g_strcmp0 (property_name, "ProtocolVersion") == 0) {
    return g_variant_new_int32 (0);
  }

  return NULL;
}

static const GDBusInterfaceVTable interface_vtable = {
  handle_method_call,
  handle_get_property,
  NULL,
};

static void on_bus_acquired (GDBusConnection* connection, const gchar* name, gpointer user_data) {
  SnBackend* backend = user_data;
  backend->connection = g_object_ref (connection);

  GBytes* xml_bytes = g_resources_lookup_data (
    SN_WATCHER_INTROSPECTION_PATH,
    G_RESOURCE_LOOKUP_FLAGS_NONE,
    NULL
  );

  if (xml_bytes == NULL) {
    g_warning ("Failed to load introspection XML from resources");
    return;
  }

  const gchar* xml_data = g_bytes_get_data (xml_bytes, NULL);

  GError* error = NULL;
  GDBusNodeInfo* node_info = g_dbus_node_info_new_for_xml (xml_data, &error);
  g_bytes_unref (xml_bytes);

  if (node_info == NULL) {
    g_warning ("Failed to parse introspection XML: %s", error->message);
    g_error_free (error);
    return;
  }

  backend->registration_id = g_dbus_connection_register_object (
    connection,
    SN_WATCHER_OBJ_PATH,
    node_info->interfaces[0],
    &interface_vtable,
    backend,
    NULL,
    &error
  );

  if (backend->registration_id == 0) {
    g_warning ("Failed to register object: %s", error->message);
    g_error_free (error);
  }
  else {
    g_debug ("StatusNotifierWatcher object exported on %s", SN_WATCHER_OBJ_PATH);
  }

  g_dbus_node_info_unref (node_info);
}

static void on_name_acquired (GDBusConnection* connection, const gchar* name, gpointer user_data) {
  g_debug ("Acquired bus name: %s", name);
}

static void on_name_lost (GDBusConnection* connection, const gchar* name, gpointer user_data) {
  SnBackend* backend = user_data;

  g_warning ("Lost (or failed to acquire) bus name: %s", name);

  if (backend->registration_id > 0 && backend->connection != NULL) {
    g_dbus_connection_unregister_object (backend->connection, backend->registration_id);
    backend->registration_id = 0;
  }
}

SnBackend* sn_backend_new (void) {
  SnBackend* backend = g_slice_new0 (SnBackend);
  return backend;
}

void sn_backend_init (SnBackend* backend) {
  g_return_if_fail (backend != NULL);

  backend->bus_name_id = g_bus_own_name (
    G_BUS_TYPE_SESSION,
    SN_WATCHER_BUS_NAME,
    G_BUS_NAME_OWNER_FLAGS_NONE,
    on_bus_acquired,
    on_name_acquired,
    on_name_lost,
    backend,
    NULL
  );
}

void sn_backend_deinit (SnBackend* backend) {
  g_return_if_fail (backend != NULL);

  if (backend->bus_name_id > 0) {
    g_bus_unown_name (backend->bus_name_id);
    backend->bus_name_id = 0;
  }

  if (backend->registration_id > 0 && backend->connection != NULL) {
    g_dbus_connection_unregister_object (backend->connection, backend->registration_id);
    backend->registration_id = 0;
  }

  if (backend->connection != NULL) {
    g_object_unref (backend->connection);
    backend->connection = NULL;
  }

  g_list_free_full (backend->registered_items, g_free);
  backend->registered_items = NULL;

  g_slice_free (SnBackend, backend);
}
