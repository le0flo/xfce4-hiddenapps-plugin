#include "dialogs.h"

#ifdef HAVE_XFCE_REVISION_H
#include "xfce-revision.h"
#endif

static void on_max_columns_changed (GtkSpinButton *spin, HiddenApps *instance);

static void configure_response (GtkWidget* dialog, gint response, HiddenApps* instance) {
  gboolean result;

  if (response == GTK_RESPONSE_HELP) {
    result = g_spawn_command_line_async ("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL);

    if (G_UNLIKELY (result == FALSE)) {
      g_warning (_ ("Unable to open the following url: %s"), PLUGIN_WEBSITE);
    }
  }
  else {
    g_object_set_data (G_OBJECT (instance->plugin), "dialog", NULL);

    config_save (instance->plugin, instance->config);

    gtk_widget_destroy (dialog);
  }
}

void dialog_configure (XfcePanelPlugin *plugin, HiddenApps *instance) {
  GtkWidget *dialog;

  if (instance->settings_dialog != NULL) {
    gtk_window_present (GTK_WINDOW (instance->settings_dialog));
    return;
  }

  /* create the dialog */
  instance->settings_dialog = dialog = xfce_titled_dialog_new_with_mixed_buttons (
    _ ("Hidden apps"),
    GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
    GTK_DIALOG_DESTROY_WITH_PARENT, "help-browser-symbolic", _ ("_Help"),
    GTK_RESPONSE_HELP, "window-close-symbolic", _ ("_Close"), GTK_RESPONSE_OK,
    NULL
  );

  g_object_add_weak_pointer (G_OBJECT (instance->settings_dialog), (gpointer*) &instance->settings_dialog);

  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-settings");

  GtkWidget *content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (box), 12);

  GtkWidget *label = gtk_label_new (_("Max columns:"));
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  GtkWidget *spin = gtk_spin_button_new_with_range (1, 20, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), instance->config->max_columns);
  g_signal_connect (spin, "value-changed", G_CALLBACK (on_max_columns_changed), instance);
  gtk_box_pack_start (GTK_BOX (box), spin, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (content), box, FALSE, FALSE, 0);
  gtk_widget_show_all (box);

  g_object_set_data (G_OBJECT (plugin), "dialog", dialog);
  g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK (configure_response), instance);

  gtk_widget_show (dialog);
}

void dialog_about (XfcePanelPlugin *plugin) {
  const gchar *auth[] = {"Leonardo <noreply@leoflo.me>", "Xfce development team <xfce4-dev@xfce.org>", NULL};

  gtk_show_about_dialog(
    NULL, "logo-icon-name", "xfce4-hiddenapps-plugin",
    "license", xfce_get_license_text (XFCE_LICENSE_TEXT_GPL),
    "version", VERSION_FULL,
    "program-name", PACKAGE_NAME,
    "comments", _("Windows like system tray plugin"),
    "website", PLUGIN_WEBSITE,
    "copyright", "Copyright \xc2\xa9 2006-" COPYRIGHT_YEAR " Leonardo",
    "authors", auth,
    NULL
  );
}

static void on_max_columns_changed (GtkSpinButton *spin, HiddenApps *instance) {
  instance->config->max_columns = gtk_spin_button_get_value_as_int (spin);
}
