# RetroSim Graphics Technical Reference

## General

- **Screen Resolution:** 480x256 pixels
- **Graphics Mode:** Palette-based with 256 colors
- **Half-res Mode:** 240x128 pixels (optional)

### Palette Memory

- RGBA (32-bit) values define the 256 colors in the palette.
- Palette memory size: 1024 bytes (256 colors * 4 bytes each).
- Palette RAM is writable and can be modified.

### Map Memory

- Defines which tiles are displayed on the map.
- Each byte represents an index into the tile memory.
- Map dimensions adjustable using MAP_WIDTH and MAP_HEIGHT.

#### Tile Count for Various Tile Sizes

| Tile Mode | Horizontal Tiles | Vertical Tiles |
| --------- | ---------------- | -------------- |
| 8x8       | 60               | 32             |
| 8x16      | 60               | 16             |
| 16x8      | 30               | 32             |
| 16x16     | 30               | 16             |

### Tile Memory

- Stores color indices for individual tiles.
- Tile size specified by TILE_WIDTH and TILE_HEIGHT registers.
- Each byte contains an index to the palette RAM (256 colors).

### Sprite Memory (Atlas)

- Contains sprite color data.
- Rectangular area of 128x128 pixels.
- Multiple sprites can be drawn by specifying the top-left corner and dimensions.

### Bitmap Memory

- Similar to sprite memory, can be up to 120K in size.
- Contains a 480x256 byte array with indices referring to the palette.

## Character Memory

- Contains the bitmap for the character sets. Binary format: 0 indicates background color, 1 indicates visible pixel.

## Memory Layout

| Address       | Size | Symbol        | Description               | Region Size (bytes) |
| ------------- | ---- | ------------- | ------------------------- | ------------------- |
| $0-$FF        |      |               | 65K vectors               | 256                 |
| $100-$1FF     | u8   |               | 65xx stack                | 256                 |
| $200-$FFF     |      |               | Free/user RAM             | 3.5K                |
| $1000-$4FFF   | u8   | MAP           | Map memory (16K)          | 16K                 |
| $5000-$8FFF   | u8   | TILES         | Tile data memory          | 16K                 |
| $9000-$CFFF   | u8   | SPRITE_ATLAS  | Sprite atlas/memory       | 16K                 |
| $D000-$DFFF   |      |               | Registers                 | 4K                  |
| $E000-$EFFF   | u32  | PALETTE       | Color palette memory      | 4K                  |
| $F000-$FEFF   |      |               | Free/user RAM             | 3.8K                  |
| $FF00-$FFFF   | u16  |               | 65xx CPU vectors          | 256                 |
| $10000-$2DFFF | u8   | BITMAP        | Bitmap memory             | 120K                |
| $30000-$3FFFF |      | CHARSET       | Character tile data       | 64K                 |
| $40000-       |      |               |                           |                     |

## Registers

| Address | Size  | Symbol                   | Description                                                        |
| ------- | ----  | ------------------------ | ------------------------------------------------------------------ |
| $D000   | u16   | SCREEN_WIDTH_U16         | Screen width (480)                                                 |
| $D002   | u16   | SCREEN_HEIGHT_U16        | Screen height (256)                                                |
| $D004   | u8    | TILE_WIDTH_U8            | Tile width (default: 8)                                            |
| $D005   | u8    | TILE_HEIGHT_U8           | Tile height (default: 8)                                           |
| $D006   | u8    | MAP_WIDTH_U8             | Map width in tiles (default: 60, max 128)                          |
| $D007   | u8    | MAP_HEIGHT_U8            | Map height in tiles (default: 16, max 128)                         |
| $D008   | u8    | SPRITE_ATLAS_PITCH_U8    | Width of the sprite atlas (default: 128)                           |
| $D009   | u16   | BITMAP_PITCH_U16         | Width of the bitmap memory (default: 480)                          |
| $D00B   | u8    | CHARACTER_COLOR_INDEX_U8 | The index of the color used for rendering text                     |
| $D100   | u32   | FIXED_FRAME_TIME_U32     | Fixed frame time in microseconds (µs): 1000000/f, f = refresh rate |
| $D104   | u32   | DELTA_TIME_U32           | The amount of time the last frame took in microseconds             |
| $D108   | u32   | FRAME_COUNTER_U32        | The number of frames that have elapsed since the start             |
| $D10C   | u8    | REFRESH_RATE_U8          | The refresh rate of the host screen                                |
| $D10E   | u8    | CURRENT_FPS_U8           | The fps calculated from the time spent in the last frame           |

## Graphics API

- `cls(color)` - clear screen with the specified color
- `clsnoclip(color)` - clear screen ignoring clipping
- `pixel(x, y, color)` - draw a pixel
- `palcolor(index, r, g, b)` - change a color in the palette
- `line(x0, y0, x1, y1, color)` - draw a line
- `circle(x, y, radius, color, filled)` - draw a circle
- `rect(x, y, width, height, color, filled)` - draw a rectangle
- `tri(x0, y0, x1, y1, x2, y2, color, filled)` - draw a triangle
- `tex(x0, y0, x1, y1, x2, y2, u0, v0, u1, v1, u2, v2)` - draw a textured triangle
- `clip(x0, y0, x1, y1)` - set clipping rectangle
- `noclip()` - disable clipping
- `setfont(width, height, offset)` - select font for `print()`
- `print(text, x, y, color, scale)` - print text on screen
- `map(x, y, mapx, mapy, width, height)` - draw the tile map on screen
- `sprite(x, y, spritex, spritey, width, height)` - draw a sprite
- `bitmap(x, y, bitmapx, bitmapy, width, height)` - draw an image

### cls()

Clears the screen. Respects the clipping area, i.e., clears only the writable pixels.

### clsnoclip()

Clears the screen, ignoring the clipping area. Faster than cls().

### pixel(x, y, color)

Draws a single pixel.

- `x, y`: screen coordinate.
- `color`: the index into the color palette.

### palcolor(index, r, g, b)

Sets an entry in the color palette to a given RGB value. The RGB value is specified in the range of 0..255.

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

### clip(x0, y0, x1, y1)

Sets the clipping region. Once set, all subsequent drawing operations will respect the selected area.

- `x0, y0`: top left corner of clipping rectangle.
- `x1, y1`: bottom right corner of the clipping rectangle.

### noclip()

Disables clipping.

### setfont(width, height, offset)

Selects the font to be used with `print`. It defines the fixed-width dimensions of the font and the byte offset in the character tile data memory (CHARSET). The character memory is 64K long and can host multiple character sets of different dimensions.

- `width, height`: the font size
- `offset`: the start of the font tile data within the CHARSET memory area, in bytes.

### print(text, x, y, color, transparent_color, scale)

Draws text on the screen at the given pixel coordinates using the system font.

- `text`: the text to be printed.
- `x, y`: screen coordinates. Can be negative, in which case pixels with negative coordinates get clipped.
- `color`: the index into the color palette.
- `scale`: an integer scaler.
- `transparent_color`: the index of the transparent color. -1 for no transparency.

### map(x, y, mapx, mapy, width, height)

Draws the tile map on the screen on the given position.

- `x, y`: top left corner of the rectangular area. Can be negative.
- `mapx, mapy`: the **tile coordinates** of the top left corner of the map area to be drawn.
- `width, height`: the width of the map to be drawn **in tiles**.

### sprite(x, y, spritex, spritey, width, height, transparent_color)

Draws a sprite on the screen. The sprite memory/atlas is of a fixed size of 128x128 pixels, so that defines the maximum size of a single sprite. Sprites can of course be smaller.

- `x, y`: screen coordinates of the top left corner of the sprite.
- `spritex, spritey`: the pixel coordinates of the sprite's top left corner.
- `width, height`: the size of the sprite in pixels.
- `transparent_color`: the index of the transparent color. -1 for no transparency.

### bitmap(x, y, bitmapx, bitmapy, width, height)

Draws an image from the bitmap area.

- `x, y`: screen coordinates of the top left corner of the image.
- `bitmapx, bitmapy`: the pixel coordinates of the image's top left corner in the bitmap area.
- `width, height`: the size of the image in pixels.
