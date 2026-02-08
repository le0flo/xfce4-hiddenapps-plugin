#ifndef __SN_ITEM_H__
#define __SN_ITEM_H__

#include <gdk/gdk.h>

typedef struct {
  gchar* category;
  gchar* id;
  gchar* title;
  gchar* status;
  gchar* window_id;

  gchar* icon_name;
  GdkPixbuf* icon_pixmap;

  gchar* overlay_icon_name;
  GdkPixbuf* overlay_icon_pixmap;

  gchar* attention_icon_name;
  GdkPixbuf* attention_icon_pixmap;
  gchar* attention_movie_name;

  gchar* tooltip_icon;
  GdkPixbuf* tooltip_icon_pixmap;
  gchar* tooltip_title;
  gchar* tooltip_description;
} SnItem;

SnItem* sn_item_new(gchar* id);

#endif
