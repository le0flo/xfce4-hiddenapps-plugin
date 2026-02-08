#ifndef __SN_ITEM_H__
#define __SN_ITEM_H__

#include <gdk/gdk.h>

#define SN_ITEM_INTERFACE_FDO "org.freedesktop.StatusNotifierItem"
#define SN_ITEM_INTERFACE_KDE "org.kde.StatusNotifierItem"

typedef struct _SnItem SnItem;

typedef void (*SnItemChangedCallback) (SnItem* item, gpointer user_data);

struct _SnItem {
  gchar* bus_name;
  gchar* obj_path;
  GDBusProxy* proxy;
  GDBusConnection* connection;

  gchar* category;
  gchar* id;
  gchar* title;
  gchar* status;
  guint32 window_id;

  gchar* icon_name;
  GdkPixbuf* icon_pixmap;

  gchar* overlay_icon_name;

  gchar* attention_icon_name;
  gchar* attention_movie_name;

  gchar* tooltip_icon_name;
  gchar* tooltip_title;
  gchar* tooltip_body;

  gboolean item_is_menu;
  gchar* menu_path;

  SnItemChangedCallback changed_callback;
  gpointer changed_callback_data;
};

SnItem* sn_item_new (GDBusConnection* connection, const gchar* bus_name, const gchar* obj_path, gboolean use_kde);
void sn_item_free (SnItem* item);

void sn_item_set_changed_callback (SnItem* item, SnItemChangedCallback cb, gpointer user_data);

void sn_item_activate (SnItem* item, gint x, gint y);
void sn_item_secondary_activate (SnItem* item, gint x, gint y);
void sn_item_context_menu (SnItem* item, gint x, gint y);
void sn_item_scroll (SnItem* item, gint delta, const gchar* orientation);

#endif
