{
  description = "A Nix flake for the xdwim project";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.05";
    flake-utils.url = "github:numtide/flake-utils";
    home-manager = {
      url = "github:nix-community/home-manager";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = { self, nixpkgs, flake-utils, home-manager }:
  flake-utils.lib.eachSystem [ "aarch64-linux" "x86_64-linux" ] (system:
  let
    name = "xdwim";
    src = ./.;
    pkgs = nixpkgs.legacyPackages.${system};

    buildInputs = with pkgs; [
      xdo
    ];

    nativeBuildInputs = with pkgs; [
      cmake
      cunit
      xdotool
      xorg.libX11
    ];

    devInputs = with pkgs; [
      gdb
      lcov
      llvmPackages.clang
      valgrind
    ] ++ nativeBuildInputs;

    xdwimPackage = pkgs.stdenv.mkDerivation {
      inherit name src nativeBuildInputs buildInputs;

      doCheck = true;

      unpackPhase = ''
      '';

      configurePhase = ''
        rm GNUmakefile
        cmake -DCMAKE_INSTALL_PREFIX="$out" .
      '';

      buildPhase = ''
        make
      '';

      checkPhase = ''
        make test ARGS=-v
      '';

      installPhase = ''
        mkdir -p $out/bin && make install
      '';
    };
  in
  {
    packages.${name} = xdwimPackage;
    packages.default = xdwimPackage;

    devShells.default = pkgs.mkShell {
      inherit buildInputs devInputs nativeBuildInputs;
    };

    homeManagerModules.xdwim = import ./module/xdwim.nix;
    homeManagerModules.default = self.homeManagerModules.xdwim;
  });
}
