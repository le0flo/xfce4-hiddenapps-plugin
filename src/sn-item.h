#ifndef __SN_ITEM_H__
#define __SN_ITEM_H__

#include <gio/gio.h>

#define SN_ITEM_INTERFACE "org.freedesktop.StatusNotifierItem"

#define SN_ITEM_INTROSPECTION_PATH "/org/freedesktop/StatusNotifierItem/org.freedesktop.StatusNotifierItem.xml"

typedef struct {
  gchar* bus_name;    /* e.g. ":1.42" or "org.app.Foo" */
  gchar* obj_path;    /* e.g. "/StatusNotifierItem"     */
  GDBusProxy* proxy;
  GDBusConnection* connection;

  gchar* category;
  gchar* id;
  gchar* title;
  gchar* status;
  guint32 window_id;

  gchar* icon_name;
  gchar* overlay_icon_name;
  gchar* attention_icon_name;
  gchar* attention_movie_name;

  gchar* tooltip_icon_name;
  gchar* tooltip_title;
  gchar* tooltip_body;

  gboolean item_is_menu;
  gchar* menu_path;
} SnItem;

SnItem* sn_item_get (GDBusConnection* connection, const gchar* bus_name, const gchar* obj_path);
void sn_item_free (SnItem* item);

void sn_item_activate (SnItem* item, gint x, gint y);
void sn_item_secondary_activate (SnItem* item, gint x, gint y);
void sn_item_context_menu (SnItem* item, gint x, gint y);
void sn_item_scroll (SnItem* item, gint delta, const gchar* orientation);

#endif
