# RetroSim

### A modern imaginary computer/console similar to Picotron, PICO-8 or TIC-80.

## Specifications

**The features are still evolving as the development progresses.**

## Scripting

* [Gravity](https://marcobambini.github.io/gravity/#/README) (easy to learn, familiar, Swift-like)
* Lua
* A65000 assembly (simulated)
* 6502 assembly (simulated)

### Callbacks

* `Start()`
* `Update(deltaTime)`
* `Timer(timerDelta)`
* `Input(inputs)`

## Graphics

* Screen resolution: **480x270** pixels. (1/4th of 1080p)
* Half-res mode? (240x135)
* Text rendering
* Sprites
* 2D geometry (line, square, triangle)
* 3D geometry (mesh)
* Line-based postprocessing effects? (blitter-like)
* Palette-based:
    * https://lospec.com/palette-list/aap-64
    * https://pixeljoint.com/forum/forum_posts.asp?TID=12795

## Text/tile-modes

* Fonts can be loaded into a designated tile-memory area.
* Set the dimensions of a virtual tile (8x8, 8x16, 16x16, etc).
* Screen-memory: array of indices into the tile-memory.
* ARGB8888 pixel format (A, R, G, B byte sequence in memory)

## Input

* Keyboard / mouse / touch
* Controller (for handhelds, for example)

## File IO

* Abstract file-system
* PNG-based image file
* Mounting of directories (local filesystem)

## Debugging Features

* Display of CPU state
* Display of memory-mapped IO registers
* Memory visualization
    * Hex / text
    * Bitmap
    * Disassembly

## Editors / OS

* Terminal
* Tile editor
* Pixel editor
* Simple windowing GUI

## Audio

* PCM Channels
* Built-in synth chip (node-based?)
* SID-chip support?

## Networking?

* Web client and server

# Building

RetroSim is written in C++ using SDL. [Xmake](https://xmake.io/) is used to manage the dependencies and for building the project.

## 3rd-party libraries
* [SDL](http://www.libsdl.org/)
* [Gravity](https://marcobambini.github.io/gravity/)

# Credits

* Written by Zoltán Majoros ([GitHub](https://github.com/arcanelab)) ([Twitter](https://twitter.com/arcanelab))