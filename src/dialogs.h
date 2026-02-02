#ifndef __DIALOGS_H__
#define __DIALOGS_H__

#include <libxfce4ui/libxfce4ui.h>

#include "plugin.h"

#define VERSION_FULL "0.0.1"
#define PLUGIN_WEBSITE "https://leoflo.me"

G_BEGIN_DECLS

void dialog_configure (XfcePanelPlugin* plugin, HiddenApps* instance);
void dialog_about (XfcePanelPlugin* plugin);

G_END_DECLS

#endif
