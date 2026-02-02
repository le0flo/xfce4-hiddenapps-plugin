#ifndef __STATUSNOTIFIER_H__
#define __STATUSNOTIFIER_H__

#include "plugin.h"

typedef struct {
  gchar *id;
  gchar *title;
  gchar *icon_name;
} SnItem;

void sn_item_free (gpointer data);

GDBusProxy* watcher_new (void);
GList* watcher_sn_items (GDBusProxy* watcher);

void on_properties_changed (GDBusProxy* proxy, GVariant* changed, GStrv invalidated, HiddenApps* instance);
void on_item_registered (GDBusProxy* proxy, gchar* sender, gchar* signal, GVariant* params, HiddenApps* instance);

#endif
