# Fuzzy Mac
![Example](./res/overview.png)

A GUI fuzzy finder for macOS(Very early release, most likely will not function properly).

## Overview

## CLI mode
```
list_of_items | FuzzyMacCLI | cmd
```

## Application Mode
Runs in background with the default `cmd-space` to open. By
default it searchs for applications under `/Applications/`. Enter `<Space>` in
order to search files under `iCloud`(this is the default, I plan to make a config file support
for any dirs to look for).

** You can quicklook files using ctrl-q**


## How the app finds files ?
In cli mode and applications finding its a very simple scoring algorithm.
for file search I used spotlight's api in order to find the files as quickly as possible.

## Config

You can configure FuzzyMac by creating a file `~/.config/FuzzyMac/config.toml`

### Default config


```toml
[colors.query_input]
selection = "#cecacd"
selection_background = "#cecacd"
text = "#575279"
background = "#f2e9e1"

[colors.results_list]
selection = "#575279"
selection_background = "#cecacd"
text = "#575279"
background = "#f2e9e1"


[mode.apps]

dirs = ["/Applications/", "/System/Applications", "/Applications/Utilities/"]

apps = ["/System/Library/CoreServices/Finder.app"]

[mode.files]

dirs = ["$HOME/Library/Mobile Documents/"]

```

if the user does not define a field, it will be defined automatically using the config
    file above.

## Compiling from source


### Requirements

Requires homebrew installation of qt6, you can install it using the command
```bash
brew install qt@6

```

### cmake
For compiling both versions
```
mkdir build
cd build
cmake ..
cmake --build .
```

## TODOs

- [x] convert to `cmake` and make it work on any mac.
- [x] make it customizable.
- [ ] Add the ability to view file path. 
- [ ] Add application icon view. 
- [ ] add application folder change watcher.



