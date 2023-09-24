# RetroSim Application Binary Interface

## 65(CE)02

`BRK`, followed by a 16-bit bytes of function ID, then a 16-bit address of the parameter struct.

Example:

```
    ldx #0  
    lda #20   
    sta $800   
    stx $801 ; x = 20  
    lda #30   
    sta $802   
    stx $803 ; y = 30  
    lda #7
    sta $804 ; color = 7
    brk 
    .word DrawPixelID ; #6   
    .word $800        ; Addr. of parameters
```

Or with macros:

```
  .movew $800, 20
  .movew $802, 30
  .moveb $804, 7
  .syscall DrawPixelID, $800
```

## A65000

`SYS`, followed by a 16-bit function ID and a 32-bit address of the parameter struct.

Example:

```
    mov.w [$800], 20 ; x
    mov.w [$802], 30 ; y
    mov.b [$804], 7  ; color
    sys
    .word DrawPixelId
    .dword $800
```

or

```
    mov.w [$800], 20 ; x
    mov.w [$802], 30 ; y
    mov.b [$804], 7  ; color
    sys DrawPixelId, $800
```
