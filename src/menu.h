#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include "plugin.h"

G_BEGIN_DECLS

void menu_build (XfcePanelPlugin* plugin, HiddenApps* instance);
void menu_refresh (HiddenApps* instance);
gboolean menu_show (GtkWidget *widget, GdkEventButton* event, HiddenApps* instance);

void on_properties_changed (GDBusProxy* proxy, GVariant* changed, GStrv invalidated, HiddenApps* instance);
void on_item_registered (GDBusProxy* proxy, gchar* sender, gchar* signal, GVariant* params, HiddenApps* instance);

G_END_DECLS

#endif
