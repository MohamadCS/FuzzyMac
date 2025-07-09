# Fuzzy Mac
![Example](./res/overview.png)

A GUI fuzzy finder for macOS(Very early release, most likely will not function properly).

## Overview

The program has two modes:
- CLI mode: Pipe an input to the cli tool and it will output the selected entry, mutliple instances allowed.
- Application Mode: Runs in background, and supports app launching only.

## Config

Still didn't create a custom config file support.

## Compiling from source

### cmake

### Requirements

Requires homebrew installation of qt6, you can install it using the command
```bash
brew install qt@6

```

For compiling both versions 
```
mkdir build
cd build
cmake ..
cmake --build .
```

## TODOs

- [x] convert to `cmake` and make it work on any mac.
- [ ] make it customizable.
- [ ] Clean the missy code. 



