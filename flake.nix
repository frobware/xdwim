{
  description = "A Nix flake for the xdwim project";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.05";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
  flake-utils.lib.eachSystem [ "aarch64-linux" "x86_64-linux" ] (system:
  let
    name = "xdwim";
    src = ./.;
    pkgs = nixpkgs.legacyPackages.${system};

    buildInputs = with pkgs; [
      cunit
      xdo
    ];

    nativeBuildInputs = with pkgs; [
      cmake
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
      inherit buildInputs devInputs;
    };
  });
}
