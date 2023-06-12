# xdwim

Make xdotool DWIM.

I use this as dumb hotkey manager on Gnome.

For example, and since time immemorial, I want `<Alt-Shift-E>` to
switch to my running Emacs. If it doesn't exist I want it to start
Emacs. If the active window is already Emacs, then I want it to switch
to the next Emacs window, if any.

# Usage

```bash
$ dconf load /org/gnome/settings-daemon/plugins/media-keys/ <<EOF
[/]
custom-keybindings=['/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/emacs/', '/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/terminal/', '/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/browser/', '/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/maxvertically/']

[custom-keybindings/emacs]
binding='<Alt><Shift>e'
command="xdwimctl emacs 'emacsclient -c --alternate-editor=\"\" --frame-parameters=\"((reverse . t))\"'"
name='Emacs'

[custom-keybindings/terminal]
binding='<Alt><Shift>n'
command="xdwimctl Gnome-terminal gnome-terminal"
name='Terminal'

[custom-keybindings/browser]
binding='<Control><Alt><Shift>n'
command="xdwimctl google-chrome google-chrome-stable"
name='Browser'

[custom-keybindings/maxvertically]
binding='<Control><Alt><Shift>v'
command="wmctrl -r :ACTIVE: -b toggle,maximized_vert"
name='MaxVertically'
EOF
```
