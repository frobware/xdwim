{ config, lib, pkgs, ... }:

with lib;

let
  cfg = config.programs.xdwim;

  shortcutModule = types.submodule {
    options = {
      binding = mkOption {
        type = types.str;
        description = "keyboard shortcut";
      };

      command = mkOption {
        type = types.str;
        description = "command to execute";
      };
    };
  };
in {
  meta.maintainers = with lib.maintainers; [ frobware ];

  options.programs.xdwim = {
    enable = mkEnableOption "xdwim configuration";

    package = mkPackageOption pkgs "xdwim" {};

    shortcuts = mkOption {
      type = with types; attrsOf shortcutModule;
      description = "specification";
      default = { };
      example = literalExpression ''
      '';
    };
  };

  config = mkIf cfg.enable {
    dconf = let dconfPath = "org/gnome/settings-daemon/plugins/media-keys"; in {
      enable = true;
      settings = {
        "${dconfPath}".custom-keybindings = forEach (attrNames cfg.shortcuts) (x: (concatStringsSep "/" [ "" dconfPath "custom-keybindings" x "" ]));
      } // mapAttrs' (n: v: nameValuePair ("${dconfPath}/custom-keybindings/${n}") { inherit (v) binding command; name = "${n}"; }) cfg.shortcuts;
    };

    systemd.user.services = mkIf cfg.enable {
      xdwim = {
        Unit = {
          Description = "xdwim service";
          PartOf = [ "graphical-session-pre.target" ];
        };
        Service = {
          ExecStart = "${cfg.package}/bin/xdwim";
          Restart = "on-abort";
        };
        Install = { WantedBy = [ "graphical-session-pre.target" ]; };
      };
    };
  };
}
