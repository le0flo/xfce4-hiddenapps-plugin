{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};

      programs = with pkgs; [
        gettext
        pkg-config
        ninja
        meson
      ];

      libs = with pkgs; [
        glib
        gtk3
        xfce4-panel
        libxfce4ui
        libxfce4util
        libdbusmenu-gtk3
      ];
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = programs;
        buildInputs = libs;

        shellHook = ''
          cat > .clangd <<EOF
          CompileFlags:
            CompilationDatabase: build/
          EOF
          mkdir -p .zed
          cat > .zed/settings.json <<EOF
          {
            "lsp": {
              "clangd": {
                "binary": {
                  "arguments": [
                    "--query-driver=${pkgs.gcc}/bin/gcc"
                  ]
                }
              }
            }
          }
          EOF

          export PS1="("devshell") $PS1";
        '';
      };

      packages.${system}.default = pkgs.stdenv.mkDerivation {
        pname = "xfce4-hiddenapps-plugin";
        version = "0.0.1";

        src = pkgs.lib.cleanSource self;

        nativeBuildInputs = programs;
        buildInputs = libs;

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
      };
    };
}
