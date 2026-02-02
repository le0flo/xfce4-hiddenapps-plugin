#ifndef __STATUSNOTIFIER_H__
#define __STATUSNOTIFIER_H__

#include <glib.h>
#include <gdk/gdk.h>

typedef struct {
  gchar* id;
  gchar* title;

  gchar* icon_name;
  GdkPixbuf* icon_pixmap;

  gchar* tooltip_title;
  gchar* tooltip_description;
} SnItem;

void sn_item_free (gpointer data);
void pixbuf_data_free (guchar* pixels, gpointer user_data);

GVariant* sn_item_get_property (const gchar* bus_name, const gchar* obj_path, const gchar* prop_name);

gchar* sn_prop_parse_string (GVariant* variant);
GdkPixbuf* sn_prop_parse_icon_pixmap (GVariant* variant);
void sn_prop_parse_tooltip (GVariant* variant, gchar** title, gchar** description);

#endif
