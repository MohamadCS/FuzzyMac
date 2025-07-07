# Fuzzy Mac

THIS PROJECT WILL ONLY WORK ON MY COMPUTER(for now) UNLESS YOU CHANGE THE MAKEFILE.

A GUI fuzzy finder for macOS(Very early release, most likely will not function properly).

## Overview

The program has two modes:
- CLI mode: Pipe an input to the cli tool and it will output the selected entry, mutliple instances allowed.
- Application Mode: Runs in background, and supports app launching only.

## Config

Still didn't create a custom config file support.

## Compiling from source
For now it requires wxWidgets in order to compile:

- For compiling the GUI version.

```cpp
make

```

- For compiling the cli version.
```cpp
make CLI=1

```


## TODOs

- [] convert to `cmake` and make it work on any mac.
- [] make it customizable.
- [] Create custom text input, just like using similar concepts like my vimz app. 
- [] Clean the missy code. 



