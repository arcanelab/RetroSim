# RetroSim

## Overview

RetroSim is an imaginary/virtual computer similar to Picotron, PICO-8 or TIC-80.

## Specifications

* Screen resolution: **480x270** pixels. (1/4th of 1080p)
* Processor: **A65000** (6502 and others are planned)
* Programming API: memory-mapped **A65000 assembly** (others are planned: LUA, Javascript, C-like, etc)

## API Features

### Graphics

* Text rendering
* Sprites
* 2D geometry (line, square, triangle)
* 3D geometry (mesh)
* Line-based postprocessing effects (blitter-like)
* Palette-based:
    * https://lospec.com/palette-list/aap-64
    * https://pixeljoint.com/forum/forum_posts.asp?TID=12795

### Text/tile-modes

* Fonts can be loaded into a designated tile-memory area.
* Set the dimensions of a virtual tile (8x8, 8x16, 16x16, etc).
* Screen-memory: array of indices into the tile-memory.
* TODO: how do we map UTF-8 codes to tiles?
* ARGB8888 pixel format (A, R, G, B byte sequence in memory)

### Audio

* PCM Channels
* Built-in synth chip (node-based?)
* SID-chip support?

## Input

* Keyboard / mouse / touch
* Controller (for handhelds, for example)

## Networking

* Web client and server

## File IO

* Abstract file-system
* PNG-based image file
* Mounting of directories (local filesystem)
* Other ideas:
    * SQLite
    * Network-based (FTP or HTTP)

# Debugging Features

* Display of CPU state
* Display of memory-mapped IO registers
* Memory visualization
    * Hex / text
    * Bitmap
    * Disassembly

# Operating System / Editors

* Simple windowing GUI
* Terminal
* Tile editor
* Pixel editor

# Scripting

* A65000 assembly (simulated)
* 6502 assembly (simulated)
* PicoC (interpreted)
* Lua (interpreted)

The interpreted languages have their heap allocated on the simulated CPU memory, this allows them to share data between the scripting languages and the CPU cores.

C's malloc gets the start memory address of the heap, as well as the size of the heap from configuration registers/memory addresses. Same for the stack. Using malloc and free are optional for scripts written in C, though, they are free to write anywhere in the valid memory range available to the virtual computer. No memory protection implemented, just as on the C64.

First the simulated CPU's will run. How much, I'm not sure yet.

As for the scripts, all loaded and enabled script files will have callbacks that the VM runtime will call into.

Idea: the runtime is implemented in A65k, running indefinitely, listening to interrupts. On vblank it signals to the VM to invoke the callbacks in the scripts.

Or the VM will keep running the virtual CPU until a timer fires (based on the framerate) and then the scripts will be called. The timer could be set to a certain percentage of the frame time. If the previous frame lasted too long, the CPU and the scripts will have less time allocated.

Perhaps it's configurable how the available CPU time will be allocated between the virtual CPU and the scripts, as well as the rendering logic/simulated hardware.

ALternative idea: first all scripts are run and if there's time left, the virtual CPU is run in the remaining time.

We could do the rendering/runtime on the main thread and the CPU/scripting on a separate thread. Let's see if the PicoC and Lua runtimes are thread-safe. During rendering/etc the scripting thread is paused.

Scripts will be stored in the VM's memory, allowing self-modiying code. (!)

## Callbacks

* OnStart
* OnUpdate (once per frame, providing deltaTime)
* OnEnable
* OnDisable (scripts can be toggled on/off during runtime.)
* OnTimer (custom time interval)
* OnInput? We could do either event-based (callback/IRQ) or register-based input. We could also do buffered input, in case more input events have arrived. Maybe an input FIFO. Or the input can be polled by the scripts, this would be probably simpler to implement.

On 65k system calls will be implemented via a dedicated SYS instruction. On 6502 one will have to use an undocumented opcode to call system functions.

## Ideas for A65k mkII

Add shader-like sub-addressings

<!-- mov r5.x, c4.w       ; d^0
mov r4.x, r4.x       ; d^1
mul r5.y, r4.x, r4.x ; d^2
mul r4.y, r4.x, r5.y ; d^3
mul r5.z, r5.y, r5.y ; d^4
mul r4.z, r4.x, r5.z ; d^5
mul r5.w, r5.y, r5.z ; d^6
mul r4.w, r4.x, r5.w ; d^7 -->

## Networking

Provide web server for:
- file access
- info on VM
- console
- debugger

Using assembly one could open a (web)socket by setting registers values for:
- IP
- port number
- mode (read or write)
- and a data register for sending/receiving data from the socket

# Cart image ("ROM")

A RetroSim cart is a collection of asset files in a zip file along with A cart.txt that contains metadata about:
- name of the cart
- name of the author, contact
- data files and the start addresses where they must be loaded
- script files that must be run on start
- a single entry point for the code (for assembly object files)

When a ROM file is loaded, its contents get mounted into the RetroSim filesystem under /rom. Not all files in a ROM archive must be referenced in the ROM.txt file. Only data files that must be loaded into memory and script files that need to run at start must be included. Other files can be loaded by the program later. If no files are specified, no code will be run once the ROM file has been loaded. In that case the console will be launched with the current directory set to the freshly loaded ROM files.

## cart.txt

    # RetroSim Cartridge / ROM file
    
    # general info - metadata
    title: Demo
    version: 1.0
    date: 21.01.2023
    author: Zoltan Majoros
    contact: www.arcanelab.com, zoltan@arcanelab.com, twitter.com/arcanelab

    # cover/title image
    cover: title.png

    # code entry address
    entry: 0xa000

    # data files
    0x1000: font.dat
    0x2000: tiles.dat
    16384: music.dat
    0xa000: code.obj

    # scripts
    gravity: main.gravity
    gravity: effects.gravity
    lua: editor.lua

### Parsing

- Key/value pairs are separated by `:`. Whitespace arund separator character is ignored, but in values they are kept.
- `\#` character denotes comment lines. Comments must be on their own lines, can't be appended after key/value pairs.
- Numeric keys: the number denotes a memory address, the value contains the path to the data file. Number detection: if the first character of the line is numeric, then it's an address line. Hex and dec numbers are supported as addresses.
- If a key-word is defined multiple times, later ones will overwrite the formerly defined ones.
- If a data file overlaps an already loaded file, it will overwrite the data in memory.
- The order of scripts will define the runtime calling order.
- `title`, `date`, `version`, `author` and `contact` fields can be shown in a GUI browser, along with a `title` image. All of these are optional, but are recommended to be provided.

### Schemas

File format schemas are versioned. Default is 1.0. Keyword: `schema`. Schemas contain a set of valid keys and valid formats for the associated values. If defined, it must be the first key. Otherwise it's ignored. (Or read the whole file and find the schema key.)

# Filesystem

When RetroSim is launched, a virtual file system is created. The `retrosim.config` file can define mount points. When RetroSim is started with a ROM file in the arguments or a ROM file is loaded via libretro, the contents of the ROM file will be mounted under /rom/rom_name/.

## RAM Disk
