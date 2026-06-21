{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation {
  pname = "xfce4-hiddenapps-plugin";
  version = "0.0.1";

  src = pkgs.lib.cleanSourceWith {
    src = ./.;
    filter = path: type: pkgs.lib.cleanSourceFilter path type &&
      baseNameOf path != "build" &&
      baseNameOf path != "result";
  };

  nativeBuildInputs = with pkgs; [
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

  installPhase = ''
    runHook preInstall

    mkdir -p $out/{lib,share}/xfce4/panel/plugins

    install -D src/libhiddenapps.so $out/lib/xfce4/panel/plugins/libhiddenapps.so
    install -D src/hiddenapps.desktop $out/share/xfce4/panel/plugins/hiddenapps.desktop

    runHook postInstall
  '';

  meta = {
    description = "Windows like system tray plugin";
    homepage = "https://codeberg.org/leoflo/xfce4-hiddenapps-plugin";
    license = pkgs.lib.licenses.gpl2Plus;
    platforms = pkgs.lib.platforms.linux;
  };
}
