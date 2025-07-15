# FuzzyMac

<p align="center">
<img src="./res/icons/icon-256x256.png" alt="FuzzyMac Icon" width="150">
</p>

<p align="center">
A fuzzy finder GUI for macOS.
</p>


## 📸 Overview

<p align="center">
<img src="./res/overview.png" alt="FuzzyMac Screenshot">
</p>


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

- 📄 Press ⌘ + Y to QuickLook a selected file.


## 🔍 How It Works
I used a simple custom scoring algorithm. 
(previous versions relied on spotlight's indexing, which was very slow for some reason).

## ⚙️ Configuration

You can configure FuzzyMac by creating a file at:

~/.config/FuzzyMac/config.toml

If a setting is not provided, it will fall back to the built-in default below.

## 🔧 Default Config

```toml
font = "JetBrainsMono Nerd Font"

[colors.query_input]
selection = "#cecacd"
selection_background = "#cecacd"
text = "#575279"
background = "#faf4ed"

[colors.results_list]
selection = "#575279"
selection_background = "#dfdad9"
text = "#575279"
background = "#faf4ed"


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
- [ ] Display full file paths in the UI.
