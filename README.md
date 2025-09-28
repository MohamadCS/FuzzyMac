# FuzzyMac

<p align="center">
<img src="./res/app_icon/icon-256x256.png" alt="FuzzyMac Icon" width="150">
</p>

<p align="center">
A fuzzy finder GUI for macOS.
</p>


## 📸 Overview

<p align="center">
<img src="./res/overview.png" alt="FuzzyMac Screenshot">
</p>


## Key Features

- Search applications, files instantly.
- File preview.
- Clipboard Manager.
- (Almost) everything is configurable using a config file.
- CLI mode: Pipe Resutls > Search > Enter.


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
```shell
xattr -d com.apple.quarantine /Applications/FuzzyMac.app

```

### 🛠️ Building from Source

#### ✅ Requirements
-	macOS with Homebrew
-	Qt 6 installed via Homebrew:
```
brew install qt@6
```

#### 🔨 Build with CMake

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

This builds both the CLI and GUI versions.


###  🖥️ CLI Mode

FuzzyMac can be used directly from the command line:

- Generate a list of entries using some command
- Pip those entries to FuzzyMac
- FuzzyMac will popup the search window
- After choosing an entry FuzzyMac prints out the result to stdout.

**Example**
```bash

app=$(fd --max-depth 1 -e app . /Applications -x realpath | FuzzyMac)

	if [ -z "$app"]; then
	exit
	fi

	open -a "$app"

```

### 🧭 Application Mode

FuzzyMac runs as a background app, activated by default with ⌘ + Space.

- By default:
*	It searches for applications in default config.
*	Pressing <Space> in the input box switches to file search mode.
*   Pressing Ctrl-o on a file in file mode will search files in this directory (non-recursive search), you can press Ctrl-b to go one level below.


## 🔍 How It Works
I used a simple custom scoring algorithm.
(previous versions relied on spotlight's indexing, which was very slow for some reason).

## ⚙️ Configuration


You can configure FuzzyMac by creating a file at:

~/.config/FuzzyMac/config.toml

**NOTE**: For now copy the config and don't rely on a default one, I will add support for a default config later.

If a setting is not provided, it will fall back to the built-in default below.

## 🔧 Default Config

```toml
font = "JetBrainsMono Nerd Font"

border_size = 0
animations = true
corner_radius = 20 
info_panel = false
opacity = 0.97

[colors]
outer_border = "#545464"
inner_border = "#dddddb"

[colors.query_input]

selection = "#545464"
selection_background = "#e2e1df"
text = "#545464"
background = "#f2f1ef"

[colors.results_list]
selection = "#545464"
selection_background = "#e2e1df"
text = "#545464"
background = "#f2f1ef"
scrollbar_color = "#545464"
scrollbar_hold_color = "#4d699b"


[colors.mode_label]
text = "#43436c"
background = "#f2f1ef"

[mode.apps]
dirs = [
  "/Applications/",
  "/System/Applications",
  "/Applications/Utilities/",
  "/System/Applications/Utilities",
]
show_icons = true
apps = [
  "/System/Library/CoreServices/Finder.app",
  "/System/Volumes/Preboot/Cryptexes/App/System/Applications/Safari.app",
]

[mode.files]
show_icons = true
dirs = [
  "$HOME/Library/Mobile Documents/com~apple~CloudDocs/",
  "$HOME/Pictures/",
]

[mode.clipboard]

blacklist = ["Bitwarden", "Ghostty", "FuzzyMac"]

[mode.apps]

dirs = [
"/Applications/",
"/System/Applications",
"/Applications/Utilities/",
"/System/Applications/Utilities",
]
show_icons = true

apps = ["/System/Library/CoreServices/Finder.app"]

[mode.files]
show_icons = true

dirs = ["$HOME/Library/Mobile Documents/com~apple~CloudDocs/"]
```


## Installation
If you have homebrew installed you can tap

```shell
brew tap MohamadCS/FuzzyMac https://github.com/MohamadCS/FuzzyMac.git
brew install --cask MohamadCS/FuzzyMac/fuzzymac

```


## 🛠️ Building from Source

### ✅ Requirements
-	macOS with Homebrew
-	Qt 6 installed via Homebrew:
```
brew install qt@6
```

### 🔨 Build with CMake

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

This builds both the CLI and GUI versions.


## TODO
- [x] Migrate to CMake and make build portable.
- [x] Add application icon view.
- [x] Add configuration support.
- [x] Watch for changes in configured application directories.
- [x] Display full file paths in the UI.
- [ ] Add clipboard history support.
- [ ] Add quick settings support.
