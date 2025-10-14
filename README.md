# FuzzyMac

<p align="center">
<img src="./res/app_icon/icon-256x256.png" alt="FuzzyMac Icon" width="150">
</p>

<p align="center">
A fuzzy finder GUI for macOS.
</p>


## Overview

<p align="center">
<img src="./res/overview.png" alt="FuzzyMac Screenshot">
</p>


## Key Features

- Search applications, files instantly.
- File preview.
- Clipboard Manager.
- (Almost) everything is configurable using a config file.
- Quick settings
- Quick connetion to bluetooth devices.


## Install 

### Precompiled binary
You can install it using homebrew

```bash
brew tap MohamadCS/FuzzyMac https://github.com/MohamadCS/FuzzyMac.git
brew install --cask MohamadCS/FuzzyMac/fuzzymac
```

**NOTE**: This will install qt@6 also, for now.

or by navigating to the releases section.
#### Damaged Binary ?
Solution
```bash
xattr -d com.apple.quarantine /Applications/FuzzyMac.app

```

### Building from Source

#### Requirements
-	macOS with Homebrew
-	Qt 6 installed via Homebrew:

```bash
brew install qt@6
```

#### Build with CMake

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

**Example**
```bash

app=$(fd --max-depth 1 -e app . /Applications -x realpath | FuzzyMac)

	if [ -z "$app"]; then
	exit
	fi

	open -a "$app"

```

### Usage

FuzzyMac runs as a background app, activated by default with ⌘ + Space.

- By default:
*	It searches for applications in default config.
*	Pressing <Space> in the input box switches to file search mode.
*   Pressing Ctrl-o on a file in file mode will search files in this directory (non-recursive search), you can press Ctrl-b to go one level below.
*   Search a script's name(in the defined directory in config), in order to run it.
*   Serach your paired bluetooth devices in order to connect/disconnect. 
*   Quickly switch wallpaper(Can also be triggered by the prefix "ww").
*   Search clipboard history(Default key is ctrl-⌘-c).


## How It Works
It uses spotlight indexing to load files, and custom algorithms for more
accurate filtering.  

## Configuration
You can configure FuzzyMac by creating a file at:

~/.config/FuzzyMac/config.toml

If some setting is not provided, it will fall back to the built-in default below.

## Default Config

```toml
font = "JetBrainsMono Nerd Font"

border_size = 2
animations = false
corner_radius = 20
info_panel = false
opacity = 1.00

[colors]
outer_border = "#4d699b"
inner_border = "#f2f1ef"

[colors.query_input]

selection = "#545464"
selection_background = "#c7d7e0"
text = "#545464"
background = "#f2f1ef"

[colors.results_list]
selection = "#545464"
selection_background = "#c7d7e0"
text = "#545464"
background = "#f2f1ef"
scrollbar_color = "#4d699b"
scrollbar_hold_color = "#4e8ca2"


[colors.mode_label]
text = "#545464"
background = "#f2f1ef"

[mode.apps]
dirs = [
  "/System/Applications",
  "/Applications/",
  "/Applications/Utilities/",
  "/System/Applications/Utilities",
]
show_icons = true
apps = ["/System/Library/CoreServices/Finder.app"]

script_paths = ["$HOME/.config/FuzzyMac/scripts/"]

[mode.files]
show_icons = true
dirs = [
  "$HOME/Library/Mobile Documents/com~apple~CloudDocs/",
  "$HOME/Pictures/",
]

[mode.clipboard]
blacklist = ["Bitwarden", "FuzzyMac", "Ghostty"]
limit = 100

[mode.wallpaper]
show_icons = true
paths = ["$HOME/Pictures/Wallpapers/"]
```




## TODO
- [x] Migrate to CMake and make build portable.
- [x] Add application icon view.
- [x] Add configuration support.
- [x] Watch for changes in configured application directories.
- [x] Display full file paths in the UI.
- [x] Add clipboard history support.
- [x] Add (partially) quick settings support.
