#ifndef __STATUSNOTIFIER_H__
#define __STATUSNOTIFIER_H__

#include "plugin.h"

GDBusProxy* watcher_new (void);
void watcher_sn_items (GDBusProxy* watcher);

void on_properties_changed (GDBusProxy* proxy, GVariant* changed, GStrv invalidated, HiddenApps* instance);
void on_item_registered (GDBusProxy* proxy, gchar* sender, gchar* signal, GVariant* params, HiddenApps* instance);

#endif
