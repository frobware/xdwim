{
  description = "Devlopement shell for the xdwim project";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        devShell = pkgs.stdenv.mkDerivation {
          name = "xdwim";

          # nativeBuildInputs: build time dependencies.
          nativeBuildInputs = with pkgs; [
            gcc
            gdb
            gnumake
            lcov
            llvmPackages.clang
            pkg-config
            valgrind
            xdotool
            xorg.libX11
          ];

          # buildInputs: for runtime dependencies.
          buildInputs = with pkgs; [
            cunit
            xdo
          ];

          shellHook = ''
            export CMAKE_BUILD_TYPE=Debug
          '';
        };
      }
    );
}
