/*  $Id$
 *
 *  Copyright (C) 2025 Leonardo <noreply@leoflo.me>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __DIALOGS_H__
#define __DIALOGS_H__

#include "plugin.h"

#ifdef HAVE_XFCE_REVISION_H
#include "xfce-revision.h"
#else
#define VERSION_FULL "0.0.1"
#endif

#define PLUGIN_WEBSITE "https://leoflo.me"

#include <libxfce4ui/libxfce4ui.h>

G_BEGIN_DECLS

void dialog_configure (XfcePanelPlugin* plugin, Plugin* sample);
void dialog_about (XfcePanelPlugin* plugin);

G_END_DECLS

#endif
