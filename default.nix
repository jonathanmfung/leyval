let
  pkgs =
    import
      # f9f0d5 is 241203
      (fetchTarball "https://github.com/NixOS/nixpkgs/archive/f9f0d5c5380be0a599b1fb54641fa99af8281539.tar.gz")
      { };
  stdenv = pkgs.clangStdenv;
in
# pkgs.llvmPackages_17.libcxxStdenv.mkDerivation
stdenv.mkDerivation {
  name = "leyval";
  src = ./.;
  nativeBuildInputs = [
    pkgs.cmake
    pkgs.mold-wrapped
  ];
  buildInputs = with pkgs; [
    spdlog
    fmt
    nlohmann_json
    catch2_3
  ];
}
