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

#include "sn-item.h"

static void pixbuf_free_data (guchar* pixels, gpointer user_data);

static void call_method_xy (SnItem* item, const gchar* method, gint x, gint y);

static gchar* get_string_prop (GDBusProxy* proxy, const gchar* prop);
static guint32 get_uint32_prop (GDBusProxy* proxy, const gchar* prop);
static gboolean get_boolean_prop (GDBusProxy* proxy, const gchar* prop);
static gchar* get_object_path_prop (GDBusProxy* proxy, const gchar* prop);

static void cache_icon_pixmap (SnItem* item);
static void cache_tooltip (SnItem* item);

static void refresh_properties (SnItem* item);

static void on_properties_changed (GDBusProxy* proxy, GVariant* changed_properties, GStrv invalidated_properties, gpointer user_data);
static void on_signal (GDBusProxy* proxy, const gchar* sender_name, const gchar* signal_name, GVariant* parameters, gpointer user_data);
static void on_proxy_ready (GObject* source, GAsyncResult* res, gpointer user_data);

static void emit_item_changed (SnItem* item);

SnItem* sn_item_new (GDBusConnection* connection, const gchar* bus_name, const gchar* obj_path, gboolean use_kde) {
  g_return_val_if_fail (connection != NULL, NULL);
  g_return_val_if_fail (bus_name != NULL, NULL);

  SnItem* item = g_slice_new0 (SnItem);

  item->connection = g_object_ref (connection);
  item->bus_name = g_strdup (bus_name);
  item->obj_path = g_strdup (obj_path ? obj_path : "/StatusNotifierItem");

  const gchar* interface_name = use_kde ? SN_ITEM_INTERFACE_KDE : SN_ITEM_INTERFACE_FDO;

  g_dbus_proxy_new (
    connection,
    G_DBUS_PROXY_FLAGS_NONE,
    NULL,
    item->bus_name,
    item->obj_path,
    interface_name,
    NULL,
    on_proxy_ready,
    item
  );

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
  g_clear_object (&item->dbus_menu);

  g_slice_free (SnItem, item);
}

void sn_item_set_changed_callback (SnItem* item, SnItemChangedCallback callback, gpointer user_data) {
  g_return_if_fail (item != NULL);

  item->changed_callback = callback;
  item->changed_callback_data = user_data;
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

GtkMenu* sn_item_get_gtk_menu (SnItem* item) {
  if (item->dbus_menu == NULL) {
    return NULL;
  }

  return GTK_MENU (item->dbus_menu);
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

static void pixbuf_free_data (guchar* pixels, gpointer user_data) {
  (void) user_data;
  g_free (pixels);
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

static void cache_icon_pixmap (SnItem* item) {
  g_clear_object (&item->icon_pixmap);

  GVariant* v = g_dbus_proxy_get_cached_property (item->proxy, "IconPixmap");
  if (v == NULL) {
    return;
  }

  GVariantIter iter;
  g_variant_iter_init (&iter, v);

  gint32 best_w = 0, best_h = 0;
  GVariant* best_data = NULL;

  gint32 w, h;
  GVariant* data;
  while (g_variant_iter_next (&iter, "(ii@ay)", &w, &h, &data)) {
    if (w * h > best_w * best_h) {
      if (best_data) {
        g_variant_unref (best_data);
      }

      best_w = w;
      best_h = h;
      best_data = data;
    }
    else {
      g_variant_unref (data);
    }
  }

  if (best_data == NULL || best_w <= 0 || best_h <= 0) {
    if (best_data) {
      g_variant_unref (best_data);
    }

    g_variant_unref (v);
    return;
  }

  gsize len;
  const guint8* argb = g_variant_get_fixed_array (best_data, &len, 1);
  gsize expected = (gsize) best_w * best_h * 4;

  if (len < expected) {
    g_variant_unref (best_data);
    g_variant_unref (v);
    return;
  }

  guint8* rgba = g_malloc (expected);

  for (gsize i = 0; i < (gsize) best_w * best_h; i++) {
    guint32 pixel = GUINT32_FROM_BE (*(const guint32*)(argb + i * 4));

    guint8 a = (pixel >> 24) & 0xff;
    guint8 r = (pixel >> 16) & 0xff;
    guint8 g = (pixel >> 8) & 0xff;
    guint8 b = (pixel >> 0) & 0xff;

    rgba[i * 4 + 0] = r;
    rgba[i * 4 + 1] = g;
    rgba[i * 4 + 2] = b;
    rgba[i * 4 + 3] = a;
  }

  item->icon_pixmap = gdk_pixbuf_new_from_data (
    rgba, GDK_COLORSPACE_RGB, TRUE, 8,
    best_w, best_h, best_w * 4,
    pixbuf_free_data, NULL
  );

  g_variant_unref (best_data);
  g_variant_unref (v);
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
  g_clear_object (&item->icon_pixmap);

  g_free (item->overlay_icon_name);

  g_free (item->attention_icon_name);
  g_free (item->attention_movie_name);

  g_free (item->menu_path);
  g_clear_object (&item->dbus_menu);

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

  if (item->menu_path != NULL && item->menu_path[0] != '\0') {
    item->dbus_menu = DBUSMENU_GTKMENU (dbusmenu_gtkmenu_new (item->bus_name, item->menu_path));
  }

  cache_icon_pixmap (item);
  cache_tooltip (item);
}

static void on_properties_changed (GDBusProxy* proxy, GVariant* changed_properties, GStrv invalidated_properties, gpointer user_data) {
  SnItem* item = user_data;

  refresh_properties (item);

  emit_item_changed (item);
}

static void on_signal (GDBusProxy* proxy, const gchar* sender_name, const gchar* signal_name, GVariant* parameters, gpointer user_data) {
  SnItem* item = user_data;

  gboolean changed = TRUE;

  if (g_strcmp0 (signal_name, "NewTitle") == 0) {
    g_free (item->title);
    item->title = get_string_prop (proxy, "Title");
  }
  else if (g_strcmp0 (signal_name, "NewIcon") == 0) {
    g_free (item->icon_name);
    item->icon_name = get_string_prop (proxy, "IconName");
    cache_icon_pixmap (item);
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
  else {
    changed = FALSE;
  }

  if (changed) {
    emit_item_changed (item);
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

  emit_item_changed (item);
}

static void emit_item_changed (SnItem* item) {
  if (item->changed_callback != NULL) {
    item->changed_callback (item, item->changed_callback_data);
  }
}
