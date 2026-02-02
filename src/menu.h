#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <libxfce4ui/libxfce4ui.h>

#include "plugin.h"

G_BEGIN_DECLS

void menu_build (XfcePanelPlugin* plugin, HiddenApps* instance);
gboolean menu_show (GtkWidget *widget, GdkEventButton* event, HiddenApps* instance);

G_END_DECLS

#endif
