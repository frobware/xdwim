# xdwim

[![Build Status](https://travis-ci.com/frobware/xdwim.svg?branch=master)](https://travis-ci.org/frobware/xdwim)

Make xdotool DWIM.

I use this as dumb hotkey manager on Gnome.

For example, and since time immemorial, I want `<Alt-Shift-E>` to
switch to my running Emacs window. If it doesn't exist I want it to
start emacs. If the active window is already emacs then I want it to
switch to the next emacs window, if any. This latter behaviour is
something I used to adore in Sawfish many, many eons ago.

This was my first Rust program -- I like what I see. As I learn a new
programming language I always (seem to) gravitate to the parts that
interact with C - Rust's FFI seems fabulous. However, I noticed that
libxdo (C library) was leaking badly (another reason I'm looking at
Rust) and raised the following PRs:

- [https://github.com/jordansissel/xdotool/pull/241](https://github.com/jordansissel/xdotool/pull/241)
- [https://github.com/jordansissel/xdotool/pull/242](https://github.com/jordansissel/xdotool/pull/242)

These are both present in my fork on the "xdwim" branch:

- [https://github.com/frobware/xdotool/tree/xdwim](https://github.com/frobware/xdotool/tree/xdwim)

# Usage

This is how I use it on Gnome with dconf:

```bash
$ dconf load /org/gnome/settings-daemon/plugins/media-keys/ <<EOF
[/]
custom-keybindings=['/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/emacs/', '/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/terminal/', '/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/browser/', '/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/gnus/', '/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/pdf/', '/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/maxvertically/']

[custom-keybindings/emacs]
binding='<Alt><Shift>e'
command="rxdwimctl emacs bash -login -c 'emacsclient -c --alternate-editor=\"\" --frame-parameters=\"((reverse . t))\"'"
name='Emacs'

[custom-keybindings/terminal]
binding='<Alt><Shift>n'
command="rxdwimctl Gnome-terminal gnome-terminal"
name='Terminal'

[custom-keybindings/browser]
binding='<Control><Alt><Shift>n'
command="rxdwimctl google-chrome google-chrome"
name='Browser'

[custom-keybindings/gnus]
binding='<Control><Alt><Shift>m'
command="rxdwimctl gnus gnus"
name='Mail'

[custom-keybindings/maxvertically]
binding='<Control><Alt><Shift>v'
command="wmctrl -r :ACTIVE: -b toggle,maximized_vert"
name='MaxVertically'
EOF
```

