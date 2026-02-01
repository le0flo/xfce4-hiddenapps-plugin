{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = {nixpkgs, ...}:
  let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
  in {
    devShells.${system}.default = pkgs.mkShell {
      packages = with pkgs; [
        gettext pkg-config
        ninja meson
      ];

      buildInputs = with pkgs; [
        glib gtk3
        xfce4-panel
        libxfce4ui libxfce4util
      ];
    };

    packages.${system}.default = pkgs.stdenv.mkDerivation {
      pname = "xfce4-hiddenapps-plugin";
      version = "0.0.1";
      src = ./.;

      nativeBuildInputs = with pkgs; [
        gettext pkg-config
        ninja meson
      ];

      buildInputs = with pkgs; [
        glib gtk3
        xfce4-panel
        libxfce4ui libxfce4util
      ];

      installPhase = ''
        mkdir -p $out/{lib,share}/xfce4/panel/plugins

        cp src/libhiddenapps.so $out/lib/xfce4/panel/plugins
        cp src/hiddenapps.desktop $out/share/xfce4/panel/plugins
      '';

      meta = {
        description = "Windows like system tray plugin";
        homepage = "https://codeberg.org/leoflo/xfce4-hiddenapps-plugin";
        license = pkgs.lib.licenses.gpl2Plus;
        platforms = pkgs.lib.platforms.linux;
      };
    };
  };
}
