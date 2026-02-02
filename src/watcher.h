#ifndef __WATCHER_H__
#define __WATCHER_H__

#include <gdk/gdk.h>

GDBusProxy* watcher_new (void);
GList* watcher_sn_items (GDBusProxy* watcher);

#endif
