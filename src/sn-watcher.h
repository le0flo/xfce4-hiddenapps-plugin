#ifndef __SN_WATCHER_H__
#define __SN_WATCHER_H__

#include <gdk/gdk.h>

typedef struct {
  GList* registered_sn_items;
  gboolean is_sn_host_registered;
  gint proto_version;
} SnWatcher;

SnWatcher* sn_watcher_new(void);

#endif
