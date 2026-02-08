#ifndef __MENU_H__
#define __MENU_H__

#include "plugin.h"

void menu_build (HiddenApps* instance);
void menu_refresh (HiddenApps* instance);

gboolean menu_show (GtkWidget *widget, GdkEventButton* event, HiddenApps* instance);

#endif
