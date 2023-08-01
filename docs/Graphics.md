# Graphics in RetroSim

## General

**Screen resolution: 480x256**

(Later: Half-res mode: 240x128)

### Map memory

A memory area containing an array of (u8) indices into the tile-memory. The tile-memory can contain any kind of bitmap data, both text and non-text characters/tiles. The tile memory is initialized with the default font data.

The element size of the map memory is configurable. By default the element size is 8-bits. However, this can be changed to 16 or 32 bits via a configuration register.

Depending on this setting, the maximum number of addressable tiles are limited. See table for details.

| Index size in bits | Addressable tiles | Size of tile memory |
| - | - | - |
| 8   | 256 | 1024 (0x400) |
| 16  | 65536 | 256Kb (40000) |
| 32  | 4294967296 | More than enough |


#### Map memory size

Depending on the selected tile mode, the length/dimension of the Map RAM changes as such:

For example, in 8x8 tile mode we have a grid of 60x32 tiles in the map RAM.

| Tile mode | Horizontal tiles | Vertical tiles|
| - | - | - |
| 8x8   | 60 | 32 |
| 8x16  | 60 | 16 |
| 16x8  | 30 | 32 |
| 16x16 | 30 | 16 |

Later a scrollable map that is larger than the window will also be supported.

### Tile memory

This is where the bitmaps for the tiles are stored. Depending on the tile mode selected, the contents of this memory area will be interpreted as tiles of the following sizes:

* 8x8
* 8x16
* 16x8
* 16x16

Each byte contains an index to the palette RAM, which contains 256 colors.

### Palette memory

This is where the RGBA (32-bit) values are defined for the 256 colors in the palette. Since a color is defined by four bytes (one bye for the reg, green, blue and alpha components each), it is 256*4 = 1024 bytes long.

The palette RAM is initialized with the default palette, but it's writeable so it can be modified.

### Sprites



## Graphics registers

|Address     | Size  |  Description
|------------|-------|------------
|$100        | u8    | Tile-mode
|            |       | 00:  8x8  (60x32 tiles, 480x256)
|            |       | 01:  8x16 (60x16 tiles, 480x256)
|            |       | 10: 16x8  (30x32 tiles, 480x256)
|            |       | 11: 16x16 (30x16 tiles, 480x256)
|$101-$104   | u32   | Map memory offset (later can divide it up to x/y, 16 bits each)
|$105-$106   | u16   | Palette memory offset
|$107        | u8    | Map memory element size. 0: 8-bit, 1: 16-bit, 2: 32-bit
|$1000-$1fff | u32   | Palette memory
|$2000-$5fff | u8    | Map memory
|$6000-$6780 | u8    | Tile memory





























<!--
## Memory Map

|Address     | Size  |  Description
|------------|-------|------------
|$100        | u8    | Tile-mode
|            |       | 00:  8x8  (60x33 tiles, 480x264)
|            |       | 01:  8x16 (60x16 tiles, 480x256)
|            |       | 10: 16x8  (30x33 tiles, 480x264)
|            |       | 11: 16x16 (30x16 tiles, 480x256)
|$101        | u8    | Window Width (in tiles, max screen size)
|$102        | u8    | Window Height (in tiles, max screen size)
|$103        | u8    | Map Width (1-128)
|$104        | u8    | Map Height (1-128)
|$105        | u8    | Map x-offset (by tiles)
|$106        | u8    | Map y-offset (by tiles)
|$107-$108   | i16   | Map x-offset (by pixels, -7..7 or -15..15, depending on tile-mode)
|$109-$10A   | i16   | Map y-offset (by pixels, -7..7 or -15..15, depending on tile-mode)
|$10B-$10C   | i16   | Window x-offset (by pixels)
|$10D-$10E   | i16   | Window y-offset (by pixels)
|$8000- | |
|$1000-$3000 | u8    | Tilemap Bank 0
|$3000-$5000 | u8    | Tilemap Bank 1
|$5000-$7000 | u8    | Tilemap Bank 2
|$7000-$9000 | u8    | Tilemap Bank 3

How to calculate tile offset by pixels with 16-wide tiles:
`finalPixelOffset = (tileOffsetX << 4) | tileOffsetPixelX`

How to calculate tile offset by pixels with 8-wide tiles:
`finalPixelOffset = (tileOffsetX << 3) | tileOffsetPixelX`


### Examples

#### Default mode

- Tile mode: `8x8`
- Tile window: `60x33`
- Tile grid: `60x33`
- Tile offset: `0,0`

#### Tile grid greater than tile window

- Tile mode: `8x8`
- Tile window: `60x33`
- Tile grid: `120x67` (2x big as the default)
- Tile offset: `30, 17` (tile window centered on the grid)

The grid is twice as wide and tall than the tile window. It expands beyond the window's dimension in all directions, as the window is positioned at the center of the grid.

#### Small tile window

- Tile mode: `8x8`
- Tile window: `30x16`
- Tile grid: `30x16`
- Tile offset: `0, 0`

The tile window occupies only the `30x16` top-left area of the screen, the rest is drawn with the background color/pattern.

---
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

-->
