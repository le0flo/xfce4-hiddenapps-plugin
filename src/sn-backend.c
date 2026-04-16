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

#include "sn-backend.h"

#include "sn-item.h"

static SnItem* find_item (SnBackend* backend, const gchar* bus_name, const gchar* obj_path);
static void parse_item_id (const gchar* item_id, const gchar* sender, gchar** out_bus_name, gchar** out_obj_path);

static void add_item (SnBackend* backend, const gchar* bus_name, const gchar* obj_path, gboolean use_kde);
static void remove_item (SnBackend* backend, const gchar* bus_name);

static void handle_method_call (GDBusConnection* connection, const gchar* sender, const gchar* object_path, const gchar* interface_name, const gchar* method_name, GVariant* parameters, GDBusMethodInvocation* invocation, gpointer user_data);
static GVariant* handle_get_property (GDBusConnection* connection, const gchar* sender, const gchar* object_path, const gchar* interface_name, const gchar* property_name, GError** error, gpointer user_data);

static guint register_interface (GDBusConnection* connection, const gchar* resource_path, SnBackend* backend);

static void on_item_changed (SnItem* item, gpointer user_data);
static void on_bus_acquired (GDBusConnection* connection, const gchar* name, gpointer user_data);
static void on_name_acquired (GDBusConnection* connection, const gchar* name, gpointer user_data);
static void on_name_lost (GDBusConnection* connection, const gchar* name, gpointer user_data);
static void on_name_vanished (GDBusConnection* connection, const gchar* name, gpointer user_data);

static void emit_backend_changed (SnBackend* backend);
static void emit_host_registered (GDBusConnection* connection);

static const GDBusInterfaceVTable interface_vtable = {
  handle_method_call,
  handle_get_property,
  NULL,
};

SnBackend* sn_backend_new (void) {
  return g_slice_new0 (SnBackend);
}

void sn_backend_init (SnBackend* backend) {
  g_return_if_fail (backend != NULL);

  backend->bus_name_id = g_bus_own_name (
    G_BUS_TYPE_SESSION,
    SN_WATCHER_BUS_NAME,
    G_BUS_NAME_OWNER_FLAGS_REPLACE | G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT,
    on_bus_acquired,
    on_name_acquired,
    on_name_lost,
    backend,
    NULL
  );

  backend->kde_bus_name_id = g_bus_own_name (
    G_BUS_TYPE_SESSION,
    SN_WATCHER_KDE_BUS_NAME,
    G_BUS_NAME_OWNER_FLAGS_REPLACE | G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT,
    NULL,
    on_name_acquired,
    on_name_lost,
    backend,
    NULL
  );
}

void sn_backend_deinit (SnBackend* backend) {
  g_return_if_fail (backend != NULL);

  if (backend->kde_bus_name_id > 0) {
    g_bus_unown_name (backend->kde_bus_name_id);
    backend->kde_bus_name_id = 0;
  }

  if (backend->bus_name_id > 0) {
    g_bus_unown_name (backend->bus_name_id);
    backend->bus_name_id = 0;
  }

  if (backend->kde_registration_id > 0 && backend->connection != NULL) {
    g_dbus_connection_unregister_object (backend->connection, backend->kde_registration_id);
    backend->kde_registration_id = 0;
  }

  if (backend->registration_id > 0 && backend->connection != NULL) {
    g_dbus_connection_unregister_object (backend->connection, backend->registration_id);
    backend->registration_id = 0;
  }

  if (backend->connection != NULL) {
    g_object_unref (backend->connection);
    backend->connection = NULL;
  }

  g_list_free_full (backend->items, (GDestroyNotify) sn_item_free);
  backend->items = NULL;

  g_slice_free (SnBackend, backend);
}

void sn_backend_set_changed_callback (SnBackend* backend, SnBackendChangedCallback callback, gpointer user_data) {
  g_return_if_fail (backend != NULL);

  backend->changed_callback = callback;
  backend->changed_callback_data = user_data;
}

static SnItem* find_item (SnBackend* backend, const gchar* bus_name, const gchar* obj_path) {
  for (GList* l = backend->items; l != NULL; l = l->next) {
    SnItem* item = l->data;

    if (g_strcmp0 (item->bus_name, bus_name) == 0 && g_strcmp0 (item->obj_path, obj_path) == 0) {
      return item;
    }
  }

  return NULL;
}

static void parse_item_id (const gchar* item_id, const gchar* sender, gchar** out_bus_name, gchar** out_obj_path) {
  if (item_id[0] == '/') {
    *out_bus_name = g_strdup (sender);
    *out_obj_path = g_strdup (item_id);
  }
  else {
    const gchar* slash = strchr (item_id, '/');

    if (slash != NULL) {
      *out_bus_name = g_strndup (item_id, slash - item_id);
      *out_obj_path = g_strdup (slash);
    }
    else {
      *out_bus_name = g_strdup (item_id);
      *out_obj_path = g_strdup ("/StatusNotifierItem");
    }
  }
}

static void add_item (SnBackend* backend, const gchar* bus_name, const gchar* obj_path, gboolean use_kde) {
  if (find_item (backend, bus_name, obj_path) != NULL) {
    return;
  }

  SnItem* item = sn_item_new (backend->connection, bus_name, obj_path, use_kde);
  sn_item_set_changed_callback (item, on_item_changed, backend);
  backend->items = g_list_append (backend->items, item);

  g_dbus_connection_emit_signal (
    backend->connection, NULL,
    SN_WATCHER_OBJ_PATH,
    SN_WATCHER_INTERFACE,
    "StatusNotifierItemRegistered",
    g_variant_new ("(s)", bus_name),
    NULL
  );

  g_debug ("StatusNotifierItem registered: %s %s", bus_name, obj_path);
  emit_backend_changed (backend);
}

static void remove_item (SnBackend* backend, const gchar* bus_name) {
  GList* l = backend->items;
  gboolean removed = FALSE;

  while (l != NULL) {
    GList* next = l->next;
    SnItem* item = l->data;

    if (g_strcmp0 (item->bus_name, bus_name) == 0) {
      g_debug ("StatusNotifierItem removed: %s %s", item->bus_name, item->obj_path);

      g_dbus_connection_emit_signal (
        backend->connection, NULL,
        SN_WATCHER_OBJ_PATH,
        SN_WATCHER_INTERFACE,
        "StatusNotifierItemUnregistered",
        g_variant_new ("(s)", item->bus_name),
        NULL
      );

      backend->items = g_list_delete_link (backend->items, l);
      sn_item_free (item);
      removed = TRUE;
    }

    l = next;
  }

  if (removed) {
    emit_backend_changed (backend);
  }
}

static void handle_method_call (GDBusConnection* connection, const gchar* sender, const gchar* object_path, const gchar* interface_name, const gchar* method_name, GVariant* parameters, GDBusMethodInvocation* invocation, gpointer user_data) {
  SnBackend* backend = user_data;

  if (g_strcmp0 (method_name, "RegisterStatusNotifierItem") == 0) {
    const gchar* service = NULL;
    g_variant_get (parameters, "(&s)", &service);

    gchar* bus_name = NULL;
    gchar* obj_path = NULL;
    parse_item_id (service, sender, &bus_name, &obj_path);

    gboolean use_kde = (g_strcmp0 (interface_name, SN_WATCHER_KDE_INTERFACE) == 0);
    add_item (backend, bus_name, obj_path, use_kde);

    g_bus_watch_name_on_connection (
      connection,
      sender,
      G_BUS_NAME_WATCHER_FLAGS_NONE,
      NULL,
      on_name_vanished,
      backend,
      NULL
    );

    g_free (bus_name);
    g_free (obj_path);

    g_dbus_method_invocation_return_value (invocation, NULL);
  }
  else if (g_strcmp0 (method_name, "RegisterStatusNotifierHost") == 0) {
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

static GVariant* handle_get_property (GDBusConnection* connection, const gchar* sender, const gchar* object_path, const gchar* interface_name, const gchar* property_name, GError** error, gpointer user_data) {
  SnBackend* backend = user_data;

  if (g_strcmp0 (property_name, "RegisteredStatusNotifierItems") == 0) {
    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));

    for (GList* l = backend->items; l != NULL; l = l->next) {
      SnItem* item = l->data;
      g_variant_builder_add (&builder, "s", item->bus_name);
    }

    return g_variant_builder_end (&builder);
  }
  else if (g_strcmp0 (property_name, "IsStatusNotifierHostRegistered") == 0) {
    return g_variant_new_boolean (TRUE);
  }
  else if (g_strcmp0 (property_name, "ProtocolVersion") == 0) {
    return g_variant_new_int32 (0);
  }

  return NULL;
}

static guint register_interface (GDBusConnection* connection, const gchar* resource_path, SnBackend* backend) {
  GBytes* xml_bytes = g_resources_lookup_data (resource_path, G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

  if (xml_bytes == NULL) {
    g_warning ("Failed to load introspection XML: %s", resource_path);
    return 0;
  }

  const gchar* xml_data = g_bytes_get_data (xml_bytes, NULL);

  GError* error = NULL;
  GDBusNodeInfo* node_info = g_dbus_node_info_new_for_xml (xml_data, &error);
  g_bytes_unref (xml_bytes);

  if (node_info == NULL) {
    g_warning ("Failed to parse introspection XML: %s", error->message);
    g_error_free (error);
    return 0;
  }

  guint reg_id = g_dbus_connection_register_object (
    connection,
    SN_WATCHER_OBJ_PATH,
    node_info->interfaces[0],
    &interface_vtable,
    backend,
    NULL,
    &error
  );

  if (reg_id == 0) {
    g_warning ("Failed to register interface %s: %s", node_info->interfaces[0]->name, error->message);
    g_error_free (error);
  }
  else {
    g_debug ("Registered interface %s on %s", node_info->interfaces[0]->name, SN_WATCHER_OBJ_PATH);
  }

  g_dbus_node_info_unref (node_info);
  return reg_id;
}

static void on_item_changed (SnItem* item, gpointer user_data) {
  SnBackend* backend = user_data;

  emit_backend_changed (backend);
}

static void on_bus_acquired (GDBusConnection* connection, const gchar* name, gpointer user_data) {
  SnBackend* backend = user_data;
  backend->connection = g_object_ref (connection);

  backend->registration_id = register_interface (connection, SN_WATCHER_INTROSPECTION_PATH, backend);
  backend->kde_registration_id = register_interface (connection, SN_WATCHER_KDE_INTROSPECTION_PATH, backend);
}

static void on_name_acquired (GDBusConnection* connection, const gchar* name, gpointer user_data) {
  g_debug ("Acquired bus name: %s", name);

  emit_host_registered (connection);
}

static void on_name_lost (GDBusConnection* connection, const gchar* name, gpointer user_data) {
  SnBackend* backend = user_data;

  g_warning ("Lost (or failed to acquire) bus name: %s", name);

  if (backend->kde_registration_id > 0 && backend->connection != NULL) {
    g_dbus_connection_unregister_object (backend->connection, backend->kde_registration_id);
    backend->kde_registration_id = 0;
  }

  if (backend->registration_id > 0 && backend->connection != NULL) {
    g_dbus_connection_unregister_object (backend->connection, backend->registration_id);
    backend->registration_id = 0;
  }
}

static void on_name_vanished (GDBusConnection* connection, const gchar* name, gpointer user_data) {
  SnBackend* backend = user_data;

  remove_item (backend, name);
}

static void emit_backend_changed (SnBackend* backend) {
  if (backend->changed_callback != NULL) {
    backend->changed_callback (backend, backend->changed_callback_data);
  }
}

static void emit_host_registered (GDBusConnection* connection) {
  static const gchar* interfaces[] = {
    "org.freedesktop.StatusNotifierWatcher",
    "org.kde.StatusNotifierWatcher",
  };

  for (gsize i = 0; i < G_N_ELEMENTS (interfaces); i++) {
    g_dbus_connection_emit_signal (
      connection, NULL,
      SN_WATCHER_OBJ_PATH,
      interfaces[i],
      "StatusNotifierHostRegistered",
      NULL, NULL
    );
  }
}
