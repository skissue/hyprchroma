{
  inputs = {
    hyprland.url = "git+https://github.com/hyprwm/Hyprland?submodules=1";

    nix-filter.url = "github:numtide/nix-filter";
  };

  outputs =
    { self
    , hyprland
    , nix-filter
    , ...
    }:
    let
      inherit (hyprland.inputs) nixpkgs;
      forHyprlandSystems = fn: nixpkgs.lib.genAttrs (builtins.attrNames hyprland.packages) (system: fn system nixpkgs.legacyPackages.${system});
    in
    {
      packages = forHyprlandSystems (system: pkgs:
        let
          hyprlandPackage = hyprland.packages.${system}.hyprland;
        in
        rec {
          hyprchroma = pkgs.gcc13Stdenv.mkDerivation {
            pname = "hyprchroma";
            version = "0.1";
            src = nix-filter.lib {
              root = ./.;
              include = [
                "src"
                ./Makefile
              ];
            };

            nativeBuildInputs = with pkgs; [ pkg-config ];
            buildInputs = [ hyprlandPackage.dev ] ++ hyprlandPackage.buildInputs;

            installPhase = ''
              mkdir -p $out/lib
              install ./out/hyprchroma.so $out/lib/libhyprchroma.so
            '';

            meta = with pkgs.lib; {
              homepage = "https://github.com/skissue/hyprchroma";
              description = "Invert the colors of specific Windows";
              license = licenses.bsd3;
              platforms = platforms.linux;
            };
          };

          default = hyprchroma;
        });

      devShells = forHyprlandSystems (system: pkgs: {
        default = pkgs.mkShell {
          name = "hyprchroma";

          nativeBuildInputs = with pkgs; [
            clang-tools_16
          ];

          inputsFrom = [ self.packages.${system}.default ];
        };
      });
    };
}
