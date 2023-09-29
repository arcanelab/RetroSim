# Instruction Encoding

## Overview

Instruction base size: 16 bits. In case the instruction operand contains a reference to registers, an extra 8 bit value follows, selecting the register(s). Then the operands follow, if there are any. Their size depends on the *opcode size* field, which means they can be either 32, 16 or 8 bits. Addresses are always 32 bit wide.

Instruction encoding:
    HI-byte   LO-byte
    RRRAAAAA-SSIIIIII

| Bit #   | symbol | description            | size in bits | # of variations | comment     | 
| -----   | ------ | -----------            | ------------ | --------------- | -------     | 
| `00-05` | **I**  | instruction code       | 6            | 64              |             | 
| `06-07` | **S**  | opcode size            | 2            | 4               | 8/16/32 bit | 
| `08-12` | **A**  | addressing mode        | 5            | 32              |             | 
| `13-15` | **R**  | register configuration | 4            | 8               |             | 

## Opcode Size

|  ss  | Size         | 
| ---- | ------------ | 
| `00` | 32 bit       | 
| `01` | 16 bit       | 
| `10` | 8  bit       | 
| `11` | - RESERVED - | 

## Addressing Mode

|  AAAAA       | Operands           | Addressing Mode   |
| ----         | --------           | ---------------   |
| `00000` (0)  | `-no operand-    ` | Implied           |
| `00001` (1)  | `Rx, const       ` | Immediate         |
| `00010` (2)  | `const           ` | Immediate         |
| `00011` (3)  | `Rx              ` | Register          |
| `00100` (4)  | `Rx, Ry          ` | Register          |
| `00101` (5)  | `[Address]       ` | Absolute          |
| `00110` (6)  | `Rx, [Address]   ` | Absolute          |
| `00111` (7)  | `[Address], Rx   ` | Absolute          |
| `01000` (8)  | `[Address], const` | Absolute          |
| `01001` (9)  | `[Rx]            ` | Register Indirect |
| `01010` (10) | `Rx, [Ry]        ` | Register Indirect |
| `01011` (11) | `[Rx], Ry        ` | Register Indirect |
| `01100` (12) | `[Rx], const     ` | Register Indirect |
| `01101` (13) | `[Rx + const]    ` | Absolute Indexed  |
| `01110` (14) | `Rx, [Ry + const]` | Absolute Indexed  |
| `01111` (15) | `[Rx + const], Ry` | Absolute Indexed  |
| `10000` (16) | `branch          ` | Relative          |
| `10001` (17) | `direct          ` | Direct            |
| `10010` (18) | `syscall         ` | Syscall           |

## Register Configuration

The *Register Configuration* describe how many registers are referenced by the operation. It also encodes whether the post-increment or pre-decrement flags are set.

With these information, we can construct a table for all possible combinations:

|  RRR       | Configuration                                        | 
| ----       | -------------                                        | 
| `000` (0)  | No register used                                     | 
| `001` (1)  | 1 register                                           | 
| `010` (2)  | 2 registers                                          | 
| `011` (3)  | 1 register, post-increment                           | 
| `100` (4)  | 2 registers, post-increment register holding address | 
| `101` (5)  | 1 register, pre-decrement                            | 
| `110` (6)  | 2 registers, pre-decrement register holding address  | 
| `111` (7)  | - reserved -                                         |

If the *Register Configuration* bits are > 0, then an extra byte follows the instuction word, specifying the *Register Selector*. This value tells which register(s) are in the operand. Values 0..15 correspond to `R0..R13, SP, PC`.

The *Register Selector* is evaluated differently in different register configurations.

If the operand contains only one reference of a register, the *Register Selector* can take the following values: 

| Register selector (extra byte) | Register | 
| ------------------------------ | -------- | 
| `00000000`                     | `R0`     | 
| `00000001`                     | `R1`     | 
| ...                            | ...      | 
| `00001101`                     | `13`     | 
| `00001110`                     | `SP`     | 
| `00001111`                     | `PC`     | 

If the operand contains two references of registers, the *Register Selector* is divided into two nibbles: the high nibble specifying one, the low nibble specifying the other register. If one of the registers is used in *indirect addressing mode*, it is always stored in the low nibble.

Let's see a few examples!

TODO: write examples

## Instruction Codes

`x = { |w|b}, y = {u|s}`

| Mnemonic | code |
| -------- | ---- |
| MOV      | $00  |
| CLR      | $01  |
| ADD      | $02  |
| SUB      | $03  |
| ADC      | $04  |
| SBC      | $05  |
| INC      | $06  |
| DEC      | $07  |
| MUL      | $08  |
| DIV      | $09  |
| AND      | $0A  |
| OR       | $0B  |
| XOR      | $0C  |
| SHL      | $0D  |
| SHR      | $0E  |
| ROL      | $0F  |
| ROR      | $10  |
| CMP      | $11  |
| SEC      | $12  |
| CLC      | $13  |
| SEI      | $14  |
| CLI      | $15  |
| PUSH     | $16  |
| POP      | $17  |
| PUSHA    | $18  |
| POPA     | $19  |
| JMP      | $1A  |
| JSR      | $1B  |
| RTS      | $1C  |
| RTI      | $1D  |
| BRK      | $1E  |
| NOP      | $1F  |
| BRA      | $20  |
| BEQ      | $21  |
| BNE      | $22  |
| BCC      | $23  |
| BCS      | $24  |
| BPL      | $25  |
| BMI      | $26  |
| BVC      | $27  |
| BVS      | $28  |
| BLT      | $29  |
| BGT      | $2A  |
| BLE      | $2B  |
| BGE      | $2C  |
| SEV      | $2D  |
| CLV      | $2E  |
| SLP      | $2F  |
| SXB      | $30  |
| SXW      | $31  |
| SYS      | $32  |
