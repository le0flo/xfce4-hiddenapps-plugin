#include <string.h>

#include "statusnotifier.h"
#include "menu.h"

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

static GVariant* sn_item_get_property (const gchar *bus_name, const gchar *obj_path, const gchar *prop_name) {
  GError *error = NULL;
  GVariant *result;
  GVariant *inner = NULL;

  result = g_dbus_connection_call_sync (
    g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL),
    bus_name, obj_path, "org.freedesktop.DBus.Properties", "Get",
    g_variant_new ("(ss)", "org.kde.StatusNotifierItem", prop_name),
    G_VARIANT_TYPE ("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error
  );

  if (result != NULL) {
    g_variant_get (result, "(v)", &inner);
    g_variant_unref (result);
  } else {
    g_error_free (error);
  }

  return inner;
}

static GdkPixbuf* sn_item_parse_icon_pixmap (GVariant *variant) {
  GVariantIter iter;
  gint32 width, height;
  GVariant *data_variant;
  GdkPixbuf *best = NULL;
  gint best_size = 0;

  g_variant_iter_init (&iter, variant);
  while (g_variant_iter_next (&iter, "(ii@ay)", &width, &height, &data_variant)) {
    if (width <= 0 || height <= 0) {
      g_variant_unref (data_variant);
      continue;
    }

    gsize data_len;
    const guint8 *data = g_variant_get_fixed_array (data_variant, &data_len, sizeof (guint8));

    if (data_len < (gsize)(width * height * 4)) {
      g_variant_unref (data_variant);
      continue;
    }

    /* ARGB32 network byte order -> RGBA for GdkPixbuf */
    guint8 *rgba = g_malloc (width * height * 4);
    for (gint i = 0; i < width * height; i++) {
      guint32 pixel = GUINT32_FROM_BE (((guint32*) data)[i]);
      rgba[i * 4 + 0] = (pixel >> 16) & 0xFF; /* R */
      rgba[i * 4 + 1] = (pixel >>  8) & 0xFF; /* G */
      rgba[i * 4 + 2] = (pixel >>  0) & 0xFF; /* B */
      rgba[i * 4 + 3] = (pixel >> 24) & 0xFF; /* A */
    }

    /* Pick the largest icon */
    if (width > best_size) {
      if (best != NULL)
        g_object_unref (best);
      best = gdk_pixbuf_new_from_data (rgba, GDK_COLORSPACE_RGB, TRUE, 8,
                                       width, height, width * 4,
                                       (GdkPixbufDestroyNotify) g_free, NULL);
      best_size = width;
    } else {
      g_free (rgba);
    }

    g_variant_unref (data_variant);
  }

  return best;
}

static void sn_item_parse_tooltip (GVariant *variant, gchar **title, gchar **description) {
  const gchar *t, *d;

  /* ToolTip is (sa(iiay)ss): icon_name, icon_pixmap[], title, description */
  g_variant_get (variant, "(&sa(iiay)&s&s)", NULL, NULL, &t, &d);

  *title = g_strdup (t);
  *description = g_strdup (d);
}

void sn_item_free (gpointer data) {
  SnItem *item = (SnItem*) data;
  g_free (item->id);
  g_free (item->title);
  g_free (item->icon_name);
  g_free (item->tooltip_title);
  g_free (item->tooltip_description);
  if (item->icon_pixmap != NULL)
    g_object_unref (item->icon_pixmap);
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

    GVariant *pixmap_prop = sn_item_get_property (bus_name, obj_path, "IconPixmap");
    if (pixmap_prop != NULL) {
      item->icon_pixmap = sn_item_parse_icon_pixmap (pixmap_prop);
      g_variant_unref (pixmap_prop);
    }

    GVariant *tooltip_prop = sn_item_get_property (bus_name, obj_path, "ToolTip");
    if (tooltip_prop != NULL) {
      sn_item_parse_tooltip (tooltip_prop, &item->tooltip_title, &item->tooltip_description);
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

void on_properties_changed (GDBusProxy *proxy, GVariant *changed, GStrv invalidated, HiddenApps* instance) {
  menu_refresh (instance);
}

void on_item_registered(GDBusProxy *proxy, gchar *sender, gchar *signal, GVariant *params, HiddenApps* instance) {
  menu_refresh (instance);
}
