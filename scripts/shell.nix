let
  pkgs =
    import
      # f9f0d5 is 241203
      (fetchTarball "https://github.com/NixOS/nixpkgs/archive/f9f0d5c5380be0a599b1fb54641fa99af8281539.tar.gz")
      { };
in
pkgs.mkShell {
  pname = "leyval_scripts";
  src = ./.;
  packages = [
    (pkgs.python3.withPackages (
      python-pkgs: with python-pkgs; [
        pandas
        matplotlib
      ]
    ))
  ];
}
