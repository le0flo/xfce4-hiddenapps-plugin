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
      packages = with pkgs; [ pkg-config ninja meson ];

      buildInputs = with pkgs; [
        at-spi2-atk atkmm harfbuzz cairo pango gdk-pixbuf
        glib gtk3
        xfce4-panel
        libxfce4ui libxfce4util
      ];
    };
  };
}
