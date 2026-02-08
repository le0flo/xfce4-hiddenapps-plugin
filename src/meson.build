sn_backend_resources = gnome.compile_resources(
  'sn-backend-resources',
  'dbus/sn-backend.gresource.xml',
  source_dir: 'dbus',
  c_name: 'sn_backend',
)

plugin_sources = [
  'sn-backend.h',
  'sn-backend.c',
  'sn-item.h',
  'sn-item.c',
  sn_backend_resources,
  'config.h',
  'config.c',
  'plugin.h',
  'plugin.c',
  'dialogs.h',
  'dialogs.c',
  'menu.h',
  'menu.c',
  xfce_revision_h,
]

plugin_install_subdir = 'xfce4' / 'panel' / 'plugins'

shared_module(
  'hiddenapps',
  plugin_sources,
  gnu_symbol_visibility: 'hidden',
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format('xfce4-hiddenapps-plugin'),
  ],
  include_directories: [
    include_directories('..'),
  ],
  dependencies: [
    glib,
    gtk,
    libxfce4panel,
    libxfce4ui,
    libxfce4util,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('libdir') / plugin_install_subdir,
)

i18n.merge_file(
  input: 'hiddenapps.desktop.in',
  output: 'hiddenapps.desktop',
  po_dir: '..' / 'po',
  type: 'desktop',
  install: true,
  install_dir: get_option('prefix') / get_option('datadir') / plugin_install_subdir,
)
