with import <nixpkgs> {};
  stdenv.mkDerivation {
  name = "xdwim";
  buildInputs = with pkgs; [
    binutils
    cargo
    gcc
    gnumake
    openssl
    pkgconfig
    rustc
    xdotool
    direnv
    gdb
    x11
    valgrind
    cmake
    clang
    clang-analyzer
    cppcheck
  ];
}
