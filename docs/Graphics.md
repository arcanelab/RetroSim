# Graphics in RetroSim

## General

**Screen resolution: 480x256**

256 Colors

(Later: Half-res mode: 240x128)

### Map memory

Each byte in the map memory is an index into the tile-memory.

The size of the map is defined by MAP_WIDTH and MAP_HEIGHT at memory 0x102 and 0x103.

| Tile mode | Horizontal tiles | Vertical tiles|
| ----- | -- | -- |
| 8x8   | 60 | 32 |
| 8x16  | 60 | 16 |
| 16x8  | 30 | 32 |
| 16x16 | 30 | 16 |

### Tile memory

This is where the bitmaps for the tiles are stored. The width and height of an individual tile is specified by graphics registers at addresses 0x100 and 0x101.

The tile-memory can contain any kind of bitmap data, both text and non-text characters/tiles. The tile memory is initialized with the default font data.

Each byte contains an index to the palette RAM, which contains 256 colors.

### Palette memory

This is where the RGBA (32-bit) values are defined for the 256 colors in the palette. Since a color is defined by four bytes (one bye for the reg, green, blue and alpha components each), it is 256*4 = 1024 bytes long.

The palette RAM is initialized with the default palette, but it's writeable so it can be modified.

### Sprite memory (atlas)

A rectangular area of size 128*128 containing an array of indices into the color palette. Can be thought of as a bitmap atlas, comprising of indices.

### Sprites

Sprites are drawn from an atlas

### Bitmap

480x256 byte array containing indices to the palette.

## Memory layout

| Address     | Size  |   Symbol           |   Description
| ----------- | ----- | ------------------ | ---------------------
|    $0-$FF   |       |                    |  65K vectors
|  $100-$1FF  | u8    |                    |  65xx stack
|  $200-$FFF  |       |                    |  Free/user RAM (3.5K)
| $1000-$1FFF | u32   | PALETTE            |  Color palette memory (4K)
| $2000-$5FFF | u8    | MAP                |  Map memory (16K)
| $6000-$9FFF | u8    | TILES              |  Tile memory bank (16K)
| $A000-$CFFF | u8    | SPRITE_ATLAS       |  Sprite atlas/memory bank (16K)
| $D000-$DFFF |       |                    |  Registers
| $E000-$FEFF |       |                    |  Free/user RAM (8K)
| $FF00-$FFFF | u16   |                    |  65xx CPU vectors
|$10000-$2DFFF| u8    | BITMAP             |  Bitmap memory (120K)
|$30000-$3FFFF|       | CHARSET            |  Character tile data (128K)
|$40000-      |       |                    |  

### Registers

| Address     | Size  |   Symbol           |   Description
| ----------- | ----- | ------------------ | ---------------------
| $D000       | u8    | TILE_WIDTH         |  Tile width (default: 8)
| $D001       | u8    | TILE_HEIGHT        |  Tile height (default: 8)
| $D002       | u8    | MAP_WIDTH          |  Map width (default: 60, max 128)
| $D003       | u8    | MAP_HEIGHT         |  Map height (default: 16, max 128)
| $D004       | u8    | PALETTE_BANK       |  Palette bank selector
| $D005       | u8    | MAP_BANK           |  Map bank selector
| $D006       | u8    | TILE_BANK          |  Tile bank selector
| $D007       | u8    | SPRITE_BANK        |  Sprite bank selector

## Graphics API

- `print(text, x, y, color, scale)`
- `cls()`
- `line(x0, y0, x1, y1, color)`
- `circle(x, y, radius, color, filled)`
- `rect(x, y, width, height, color, filled)`
- `tri(x0, y0, x1, y1, x2, y2, color, filled)`
- `tex(x0, y0, x1, y1, x2, y2, u0, v0, u1, v1, u2, v2)`
- `pixel(x, y, color)`
- `clip(x0, y0, x1, y1)`
- `noclip()`
- `map(x, y, mapx, mapy, width, height)`
- `sprite(x, y, spritex, spritey, width, height)`
- `palette(bank)`
- `tiles(bank)`
- `sprite(bank)`
- `palcolor(index, r, g, b)`
- `bitmap(x, y, bitmapx, bitmapy, width, height)`

### print(text, x, y, color, scale)

- text: the text to be printed.
- x, y: screen coordinates. Can be negative, in which case pixels with negative coordinates get clipped.
- color: the index into the color palette.
- scale: an integer scaler.

### cls()

Clears the screen.

### line(x0, y0, x1, y1, color)

Draws a line.

- `x0, y0`: start screen coordinates.
- `x1, y1`: end screen coordinates.
- `color`: the index into the color palette.

### circle(x, y, radius, color, filled)

Draws a circle.

- `x, y`: screen coordinates of the circle's origin.
- `radius`: the radius of the circle in pixels.
- `color`: the index into the color palette.
- `filled`: whether to fill the area with the selected color.

### rect(x, y, width, height, color, filled)

Draws a rectangle.

- `x, y`: screen coordinates of the top left corner of the rectangle.
- `width, height`: the size of the rectangle in pixels.
- `color`: the index into the color palette.
- `filled`: whether to fill the area with the selected color.

### tri(x0, y0, x1, y1, x2, y2, color, filled)

Draws a triangle.

- `x0, y0`: coordinates of the first point.
- `x1, y1`: coordinates of the second point.
- `x2, y2`: coordinates of the third point.
- `color`: the index into the color palette.
- `filled`: whether to fill the area with the selected color.

### tex(x0, y0, x1, y1, x2, y2, u0, v0, u1, v1, u2, v2, sprite)

Draws a textured triangle. The texture is taken from either the sprite or bitmap area.

- `x0, y0`: coordinates of the first point.
- `x1, y1`: coordinates of the second point.
- `x2, y2`: coordinates of the third point.
- `u0, v0`: coordinates of the first point.
- `u1, v1`: coordinates of the second point.
- `u2, v2`: coordinates of the third point.
- `sprite` [bool]: if the source should be the sprite atlas. If false, the source is the bitmap area.

### pixel(x, y, color)

Draws a single pixel.

- `x, y`: screen coordinate.
- `color`: the index into the color palette.

### clip(x0, y0, x1, y1)

Sets the clipping region. Once set, all subsequent drawing operations will respect the selected area.

- `x0, y0`: top left corner of clipping rectangle.
- `x1, y1`: bottom right corner of the clipping rectangle.

### noclip()

Disables clipping.

### map(x, y, mapx, mapy, width, height)

Draws the tile map on the screen on the given position.

- `x, y`: top left corner of the rectangular area. Can be negative.
- `mapx, mapy`: the **tile coordinates** of the top left corner of the map area to be drawn.
- `width, height`: the width of the map to be drawn **in tiles**.

### sprite(x, y, spritex, spritey, width, height)

Draws a sprite on the screen. The sprite memory/atlas is of a fixed size of 128x128 pixels, so that defines the maximum size of a single sprite. Sprites can of course be smaller.

- `x, y`: screen coordinates of the top left corner of the sprite.
- `spritex, spritey`: the pixel coordinates of the sprite's top left corner.
- `width, height`: the size of the sprite in pixels.

### palette(bank)

Selects the palette bank.

### tiles(bank)

Selects the tile bank.

### sprite(bank)

Selects the sprite bank.

### palcolor(index, r, g, b)

Sets an entry in the color palette to a given RGB value. The RGB value is specified in the range of 0..255.

### bitmap(x, y, bitmapx, bitmapy, width, height)

Same as sprite(), but works on the bitmap area.

















<!-- 
|            |       | 00:  8x8  (60x32 tiles, 480x256)
|            |       | 01:  8x16 (60x16 tiles, 480x256)
|            |       | 10: 16x8  (30x32 tiles, 480x256)
|            |       | 11: 16x16 (30x16 tiles, 480x256)
 -->

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
