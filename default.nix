let
  nixpkgs = fetchTarball "https://github.com/NixOS/nixpkgs/tarball/nixos-24.11";
  pkgs = import nixpkgs {
    config = { };
    overlays = [ ];
  };
  stdenv = pkgs.clangStdenv;
in
# pkgs.llvmPackages_17.libcxxStdenv.mkDerivation
stdenv.mkDerivation {
  name = "leyval";
  src = ./.;
  nativeBuildInputs = [ pkgs.cmake ];
  buildInputs = with pkgs; [
    spdlog
    fmt
    nlohmann_json
  ];

  shellHook = ''
    export CPATH="$NIX_CFLAGS_COMPILE";
  '';
}
