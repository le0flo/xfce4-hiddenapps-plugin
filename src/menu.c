#include "menu.h"

#include "sn-item.h"
#include "config.h"

static void menu_reposition (HiddenApps* instance);

static void on_popup_focus_out (GtkWidget* widget, GdkEventFocus* event, gpointer user_data);
static void on_icon_clicked (GtkButton* button, gpointer user_data);
static gboolean on_icon_button_press (GtkWidget* widget, GdkEventButton* event, gpointer user_data);

static GtkWidget* create_icon_widget (SnItem* item);

gboolean menu_show (GtkWidget* widget, GdkEventButton* event, HiddenApps* instance) {
  if (event->button != 1) {
    return FALSE;
  }

  gboolean visible = gtk_widget_get_visible (instance->menu);
  if (visible) {
    gtk_widget_hide (instance->menu);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (instance->item_button), FALSE);
    return TRUE;
  }

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (instance->item_button), TRUE);
  menu_refresh (instance);
  menu_reposition (instance);

  gtk_widget_show (instance->menu);
  gtk_window_present (GTK_WINDOW (instance->menu));

  return TRUE;
}

void menu_build (HiddenApps* instance) {
  instance->menu = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_type_hint (GTK_WINDOW (instance->menu), GDK_WINDOW_TYPE_HINT_POPUP_MENU);
  gtk_window_set_decorated (GTK_WINDOW (instance->menu), FALSE);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (instance->menu), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (instance->menu), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (instance->menu), FALSE);

  g_signal_connect (instance->menu, "focus-out-event", G_CALLBACK (on_popup_focus_out), instance);
}

void menu_refresh (HiddenApps* instance) {
  GtkWidget* child = gtk_bin_get_child (GTK_BIN (instance->menu));

  if (child != NULL) {
    gtk_widget_destroy (child);
  }

  GList* items = instance->backend->items;

  if (items == NULL) {
    GtkWidget* label = gtk_label_new ("No status items");

    gtk_widget_set_margin_start (label, 8);
    gtk_widget_set_margin_end (label, 8);
    gtk_widget_set_margin_top (label, 8);
    gtk_widget_set_margin_bottom (label, 8);

    gtk_container_add (GTK_CONTAINER (instance->menu), label);
    gtk_widget_show_all (instance->menu);

    return;
  }

  GtkWidget* grid = gtk_grid_new ();

  gtk_grid_set_column_spacing (GTK_GRID (grid), 2);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 2);

  gtk_widget_set_margin_start (grid, 4);
  gtk_widget_set_margin_end (grid, 4);
  gtk_widget_set_margin_top (grid, 4);
  gtk_widget_set_margin_bottom (grid, 4);

  gint index = 0;
  for (GList* l = items; l != NULL; l = l->next, index++) {
    SnItem* item = l->data;

    GtkWidget* icon = create_icon_widget (item);
    gtk_widget_set_margin_start (icon, 8);
    gtk_widget_set_margin_end (icon, 8);
    gtk_widget_set_margin_top (icon, 8);
    gtk_widget_set_margin_bottom (icon, 8);

    GtkWidget* button = gtk_button_new ();
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    gtk_container_add (GTK_CONTAINER (button), icon);

    const gchar* tip = NULL;

    if (item->tooltip_title != NULL && item->tooltip_title[0] != '\0') {
      tip = item->tooltip_title;
    }
    else if (item->title != NULL && item->title[0] != '\0') {
      tip = item->title;
    }
    else if (item->id != NULL && item->id[0] != '\0') {
      tip = item->id;
    }

    if (tip != NULL) {
      gtk_widget_set_tooltip_text (button, tip);
    }

    g_signal_connect (button, "clicked", G_CALLBACK (on_icon_clicked), item);
    g_signal_connect (button, "button-press-event", G_CALLBACK (on_icon_button_press), item);

    gint col = index % DEFAULT_CONFIG_MAX_COLUMNS;
    gint row = index / DEFAULT_CONFIG_MAX_COLUMNS;

    if (instance->config != NULL) {
      col = index % instance->config->max_columns;
      row = index / instance->config->max_columns;
    }

    gtk_grid_attach (GTK_GRID (grid), button, col, row, 1, 1);
  }

  gtk_container_add (GTK_CONTAINER (instance->menu), grid);
  gtk_widget_show_all (grid);

  if (gtk_widget_get_visible (instance->menu)) {
    menu_reposition (instance);
  }
}

static void menu_reposition (HiddenApps* instance) {
  GtkWidget* widget = instance->item_button;

  GdkWindow* window = gtk_widget_get_window (widget);

  if (window == NULL) {
    return;
  }

  gint wx, wy;
  gdk_window_get_origin (window, &wx, &wy);

  GtkAllocation alloc;
  gtk_widget_get_allocation (widget, &alloc);

  gtk_widget_realize (instance->menu);

  gint pw, ph;
  gtk_window_get_size (GTK_WINDOW (instance->menu), &pw, &ph);

  GtkOrientation orientation = xfce_panel_plugin_get_orientation (instance->plugin);
  gint x, y;

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    x = wx + alloc.x + (alloc.width - pw) / 2;
    y = wy + alloc.y - ph;

    if (y < 0) {
      y = wy + alloc.y + alloc.height;
    }
  } else {
    x = wx + alloc.x + alloc.width;
    y = wy + alloc.y;

    GdkDisplay* display = gdk_display_get_default ();
    GdkMonitor* monitor = gdk_display_get_primary_monitor (display);

    if (monitor == NULL) {
      monitor = gdk_display_get_monitor (display, 0);
    }

    GdkRectangle geom;
    gdk_monitor_get_geometry (monitor, &geom);

    if (x + pw > geom.x + geom.width) {
      x = wx + alloc.x - pw;
    }
  }

  gtk_window_move (GTK_WINDOW (instance->menu), x, y);
}

static void on_popup_focus_out (GtkWidget* widget, GdkEventFocus* event, gpointer user_data) {
  HiddenApps* instance = user_data;

  gtk_widget_hide (instance->menu);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (instance->item_button), FALSE);
}

static void on_icon_clicked (GtkButton* button, gpointer user_data) {
  SnItem* item = user_data;

  sn_item_activate (item, 0, 0);
}

static gboolean on_icon_button_press (GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
  SnItem* item = user_data;

  if (event->button == 3) {
    sn_item_context_menu (item, (gint) event->x_root, (gint) event->y_root);
    return TRUE;
  }

  return FALSE;
}

static GtkWidget* create_icon_widget (SnItem* item) {
  GtkWidget* image = NULL;

  if (item->icon_name != NULL && item->icon_name[0] != '\0') {
    image = gtk_image_new_from_icon_name (item->icon_name, GTK_ICON_SIZE_LARGE_TOOLBAR);
  } else if (item->icon_pixmap != NULL) {
    gint w, h;
    gtk_icon_size_lookup (GTK_ICON_SIZE_LARGE_TOOLBAR, &w, &h);

    GdkPixbuf* scaled = gdk_pixbuf_scale_simple (item->icon_pixmap, w, h, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf (scaled);

    g_object_unref (scaled);
  } else {
    image = gtk_image_new_from_icon_name ("application-x-executable", GTK_ICON_SIZE_LARGE_TOOLBAR);
  }

  return image;
}
