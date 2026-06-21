{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = with pkgs; [
    gettext
    pkg-config
    ninja
    meson
  ];

  buildInputs = with pkgs; [
    glib
    gtk3
    xfce4-panel
    libxfce4ui
    libxfce4util
    libdbusmenu-gtk3
  ];
}
