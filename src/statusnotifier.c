#include <string.h>

#include "statusnotifier.h"

void sn_item_free (gpointer data) {
  SnItem *item = (SnItem*) data;

  g_free (item->id);
  g_free (item->title);

  g_free (item->icon_name);
  if (item->icon_pixmap != NULL) g_object_unref (item->icon_pixmap);

  g_free (item->tooltip_title);
  g_free (item->tooltip_description);

  g_free (item);
}

void pixbuf_data_free (guchar* pixels, gpointer user_data) {
  g_free (pixels);
}

GVariant* sn_item_get_property (const gchar* bus_name, const gchar* obj_path, const gchar* prop_name) {
  GError* error = NULL;
  GVariant* inner = NULL;

  GVariant* result = g_dbus_connection_call_sync (
    g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL),
    bus_name, obj_path, "org.freedesktop.DBus.Properties", "Get",
    g_variant_new ("(ss)", "org.kde.StatusNotifierItem", prop_name),
    G_VARIANT_TYPE ("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error
  );

  if (result != NULL) {
    g_variant_get (result, "(v)", &inner);
    g_variant_unref (result);
  }
  else {
    g_error_free (error);
  }

  return inner;
}

gchar* sn_prop_parse_string (GVariant* variant) {
  gchar* result = g_strdup (g_variant_get_string (variant, NULL));

  return result;
}

GdkPixbuf* sn_prop_parse_icon_pixmap (GVariant* variant) {
  GVariantIter iter;
  gint32 width, height;
  GVariant* data_variant;

  gint best_size = 0;
  GdkPixbuf* best = NULL;

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

    guchar* rgba = g_malloc (width * height * 4);

    for (gint i = 0; i < width * height; i++) {
      guint32 pixel;

      memcpy (&pixel, data + i * 4, sizeof (guint32));

      pixel = GUINT32_FROM_BE (pixel);

      rgba[i * 4 + 0] = (pixel >> 16) & 0xFF; // R
      rgba[i * 4 + 1] = (pixel >>  8) & 0xFF; // G
      rgba[i * 4 + 2] = (pixel >>  0) & 0xFF; // B
      rgba[i * 4 + 3] = (pixel >> 24) & 0xFF; // A
    }

    if (width > best_size) {
      if (best != NULL) g_object_unref (best);

      best = gdk_pixbuf_new_from_data (
        rgba, GDK_COLORSPACE_RGB, TRUE, 8,
        width, height, width * 4,
        pixbuf_data_free, NULL
      );

      best_size = width;
    }
    else {
      g_free (rgba);
    }

    g_variant_unref (data_variant);
  }

  return best;
}

void sn_prop_parse_tooltip (GVariant* variant, gchar** title, gchar** description) {
  const gchar* t;
  const gchar* d;

  g_variant_get (variant, "(&sa(iiay)&s&s)", NULL, NULL, &t, &d);

  *title = g_strdup (t);
  *description = g_strdup (d);
}
