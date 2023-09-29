## Instruction Set

Below you can find all instructions with all possible addressing modes. The **A65000** instruction set is designed to be very orthogonal: you can use any combination of instructions/addressing modes. This way you don't have to memorize exceptions and it also gives a wide degree of freedom while programming.

Next to the 14 general purpose registers (`R0..R13`), you can directly access/modify the Program Counter and the Stack Pointer with every instructions that accepts registers as operands.

| Mnemonic  | operand(s)             | Function                | Cycles |  
| --------  | ----------             | --------                | ------ |  
| **MOV**   | `Rx, constant`         | ` Rx = constant      `  | 1      |
| **MOV**   | `Rx, Ry`               | ` Rx = Ry            `  | 1      |  
| **MOV**   | `Rx, [Address]`        | ` Rx = mem[Address]  `  | 2      |  
| **MOV**   | `Rx, [Ry]`             | ` Rx = mem[Ry]       `  | 2      |  
| **MOV**   | `[Address], Rx`        | ` mem[Address] = Rx  `  | 2      |  
| **MOV**   | `[Address], const`     | ` mem[Address] = const` | 2      |  
| **MOV**   | `[Rx], Ry`             | ` mem[Rx] = Ry       `  | 2      |  
| **MOV**   | `Rx, [Ry + const]`     | ` Rx = mem[Ry + const]` | 3      |  
| **MOV**   | `[Rx + const], Ry`     | ` mem[Rx + const] = Ry` | 3      |  
| **MOV**   | `[Rx + const], const`  | ` mem[Rx + const] = Ry` | 3      |  
|           |                        |                         |        |  
| **CLR**   | `Rx`                   | ` Rx = 0             `  | 1      |  
| **CLR**   | `[Rx]`                 | ` mem[Rx] = 0        `  | 2      |  
| **CLR**   | `[Address]`            | ` mem[Address] = 0   `  | 2      |  
| **CLR**   | `[Rx + const]`         | ` mem[Rx + const] = 0`  | 3      |  
|           |                        |                         |        |  
| **ADD**   | `Rx, constant`         | ` Rx += constant     `  | 1      |  
| **ADD**   | `Rx, Ry`               | ` Rx += Ry           `  | 1      |  
| **ADD**   | `Rx, [Address]`        | ` Rx += mem[Address] `  | 2      |  
| **ADD**   | `Rx, [Rx]`             | ` Rx += mem[Rx]      `  | 2      |  
| **ADD**   | `[Address], Rx`        | ` mem[Address] += Rx `  | 3      |  
| **ADD**   | `[Address], const`     | ` mem[Address] += const `| 3      |  
| **ADD**   | `[Rx], Rx`             | ` mem[Rx] += Rx      `  | 3      |  
| **ADD**   | `Rx, [Rx + const]`     | `Rx += mem[Rx + const]` | 3      |  
| **ADD**   | `[Rx + const], Rx`     | `mem[Rx + const] += Rx` | 4      |  
|           |                        |                         |        |  
| **SUB**   | `Rx, constant`         | ` Rx -= constant     `  | 1      |  
| **SUB**   | `Rx, Ry`               | ` Rx -= Ry           `  | 1      |  
| **SUB**   | `Rx, [Address]`        | ` Rx -= mem[Address] `  | 2      |  
| **SUB**   | `Rx, [Rx]`             | ` Rx -= mem[Rx]      `  | 2      |  
| **SUB**   | `[Address], Rx`        | ` mem[Address] -= Rx `  | 3      |  
| **SUB**   | `[Rx], Rx`             | ` mem[Rx] -= Rx      `  | 3      |  
| **SUB**   | `Rx, [Rx + const]`     | `Rx -= mem[Rx + const]` | 3      |  
| **SUB**   | `[Rx + const], Rx`     | `mem[Rx + const] -= Rx` | 4      |  
|           |                        |                         |        |  
| **INC**   | `Rx`                   | ` Rx++               `  | 1      |  
| **INC**   | `[Rx]`                 | ` mem[Rx]++          `  | 3      |  
| **INC**   | `[Address]`            | ` mem[Address]++     `  | 3      |  
| **INC**   | `[Rx + const]`         | ` mem[Rx + const]++  `  | 4      |  
|           |                        |                         |        |  
| **DEC**   | `Rx`                   | ` Rx––               `  | 1      |  
| **DEC**   | `[Rx]`                 | ` mem[Rx]––          `  | 3      |  
| **DEC**   | `[Address]`            | ` mem[Address]––     `  | 3      |  
| **DEC**   | `[Rx + const]`         | ` mem[Rx + const]––  `  | 4      |  
|           |                        |                         |        |  
| **DIV**   | ` Rx, constant`        | ` Rx / constant,        Rx = quotient, R13 = remainder`| 1 |  
| **DIV**   | ` Rx, Ry`              | ` Rx / Ry,              Rx = quotient, R13 = remainder`| 1 |  
| **DIV**   | ` Rx, [Address]`       | ` Rx / mem[Address],    Rx = quotient, R13 = remainder`| 2 |  
| **DIV**   | ` Rx, [Ry]`            | ` Rx / mem[Ry],         Rx = quotient, R13 = remainder`| 2 |  
| **DIV**   | ` [Address], Rx`       | ` mem[Address] / Rx,    [Address] = quotient, R13 = remainder`| 3 |  
| **DIV**   | ` [Rx], Ry`            | ` mem[Rx] / Ry,         [Rx] = quotient, R13 = remainder`     | 3 |  
| **DIV**   | ` Rx, [Ry + const]`    | ` Rx / mem[Ry + const], Rx = quotient, R13 = remainder`       | 3 |  
| **DIV**   | ` [Rx + const], Ry`    | ` mem[Rx + const] / Ry, [Rx+const] = quotient, R13 = remainder`| 4 |  
|           |                        |                         |       |  
| **MUL**   | ` Rx, constant`        | ` Rx * constant,       Rx = Result low 32 bits, R13 = Result high 32 bits`         | 1 |  
| **MUL**   | ` Rx, Ry`              | ` Rx * Ry,             Rx = Result low 32 bits, R13 = Result high 32 bits`         | 1 |  
| **MUL**   | `Rx, [Address]`        | ` Rx * mem[Address],   Rx = Result low 32 bits, R13 = Result high 32 bits`         | 2 |  
| **MUL**   | `Rx, [Ry]`             | ` Rx * mem[Ry],        Rx = Result low 32 bits, R13 = Result high 32 bits`         | 2 |  
| **MUL**   | `[Address], Rx`        | ` mem[Address] * Rx,   [Address] = Result low 32 bits, R13 = Result high 32 bits`  | 3 |  
| **MUL**   | `[Rx], Ry`             | ` mem[Rx] * Ry,        [Rx] = Result low 32 bits, R13 = Result high 32 bits`       | 3 |  
| **MUL**   | ` Rx, [Ry + const]`    | ` Rx * mem[Ry + const], Rx = Result low 32 bits, R13 = Result high 32 bits`        | 3 |  
| **MUL**   | ` [Rx + const], Ry`    | ` mem[Rx + const] * Ry, [Rx+const] = Result low 32 bits, R13 = Result high 32 bits`| 4 |  
|           |                        |                                            |   |  
| **AND**   | `Rx, constant`         | ` Rx &= constant     `                     | 1 |  
| **AND**   | `Rx, Ry`               | ` Rx &= Ry           `                     | 1 |  
| **AND**   | `Rx, [Address]`        | ` Rx &= mem[Address] `                     | 2 |  
| **AND**   | `Rx, [Ry]`             | ` Rx &= mem[Ry]      `                     | 2 |  
| **AND**   | `[Address], Rx`        | ` mem[Address] &= Rx `                     | 3 |  
| **AND**   | `[Rx], Ry`             | ` mem[Rx] &= Ry      `                     | 3 |  
| **AND**   | `Rx, [Ry + const]`     | ` Rx &= mem[Ry + const]`                   | 3 |  
| **AND**   | `[Rx + const], Ry`     | ` mem[Rx + const] &= Ry`                   | 4 |  
|           |                        |                                            |   |  
| **OR**    | `Rx, constant`         | ` Rx OR= constant;  `                      | 1 | 
| **OR**    | `Rx, Ry`               | ` Rx                  OR= Ry;          `   | 1 | 
| **OR**    | `Rx, [Address]`        | ` Rx                  OR= mem[Address] `   | 2 | 
| **OR**    | `Rx, [Rx]`             | ` Rx                  OR= mem[Rx]      `   | 2 | 
| **OR**    | `[Address], Rx`        | ` mem[Address]        OR= Rx `             | 3 | 
| **OR**    | `[Rx], Rx`             | ` mem[Rx]             OR= Rx      `        | 3 | 
| **OR**    | `Rx, [Ry + const]`     | ` Rx                  OR= mem[Ry + const]` | 3 | 
| **OR**    | `[Rx + const], Ry`     | ` mem[Rx + const]     OR= Ry`              | 4 | 
|           |                        |                       |     |  
| **XOR**   | `Rx, constant`         | ` Rx ^= constant       `| 1 |  
| **XOR**   | `Rx, Ry`               | ` Rx ^= Ry             `| 1 |  
| **XOR**   | `Rx, [Address]`        | ` Rx ^= mem[Address]   `| 2 |  
| **XOR**   | `Rx, [Rx]`             | ` Rx ^= mem[Rx]        `| 2 |  
| **XOR**   | `[Address], Rx`        | ` mem[Address] ^= Rx   `| 3 |  
| **XOR**   | `[Rx], Rx`             | ` mem[Rx] ^= Rx        `| 3 |  
| **XOR**   | `Rx, [Ry + const]`     | ` Rx ^= mem[Ry + const]`| 3 |  
| **XOR**   | `[Rx + const], Ry`     | ` mem[Rx + const] ^= Ry`| 4 |  
|           |                        |                         |   |  
| **SHL**   | `Rx, constant`         | ` Rx = (Rx << constant)                     ` | 1 |  
| **SHL**   | `Rx, Ry`               | ` Rx = Rx << Ry                             ` | 1 |  
| **SHL**   | `Rx, [Address]`        | ` Rx = Rx << mem[Address]                   ` | 2 |  
| **SHL**   | `Rx, [Ry]`             | ` Rx = Rx << mem[Ry]                        ` | 2 |  
| **SHL**   | `[Address], Rx`        | ` mem[Address] = (mem[Address] << Rx)       ` | 3 |  
| **SHL**   | `[Rx], Ry`             | ` mem[Rx] = (mem[Rx] << Ry)                 ` | 3 |  
| **SHL**   | `Rx,[Ry + const]`      | ` Rx = (Rx << mem[Ry + const])              ` | 3 |  
| **SHL**   | `[Rx + const], Ry`     | ` mem[Rx + const] = (mem[Rx + const] << Ry) ` | 4 |  
|           |                        |                                               |   |  
| **SHR**   | `Rx, constant`         | ` Rx = (Rx >> Ry)                           ` | 1 |  
| **SHR**   | `Rx, Ry`               | ` Rx = Rx >> Ry                             ` | 1 |  
| **SHR**   | `Rx, [Address]`        | ` Rx = Rx >> mem[Address]                   ` | 2 |  
| **SHR**   | `Rx, [Ry]`             | ` Rx = Rx >> mem[Ry]                        ` | 2 |  
| **SHR**   | `[Address], Rx`        | ` mem[Address] = (mem[Address] >> Rx)       ` | 3 |  
| **SHR**   | `[Rx], Ry`             | ` mem[Rx] = (mem[Rx] >> Ry)                 ` | 3 | 
| **SHR**   | `Rx, [Ry + const]`     | ` Rx = Rx >> (mem[Ry + const])              ` | 3 |
| **SHR**   | `[Rx + const], Ry`     | ` mem[Rx + const] = (mem[Rx + const] >> Ry) ` | 4 |  
|           |                        |                                               |   |  
| **ROL**   | `Rx, constant`         | ` Bitwise rotate left Rx by 'constant' amount of steps      ` | 1 |  
| **ROL**   | `Rx, Ry`               | ` Bitwise rotate left Rx by Ry amount of steps              ` | 1 |  
| **ROL**   | `[Rx], Ry`             | ` Bitwise rotate left mem[Rx]  by Ry amount of steps        ` | 3 |  
| **ROL**   | `[Address], Ry`        | ` Bitwise rotate left mem[Address] by Ry amount of steps    ` | 3 |  
| ..3x..    |                        |                                                               |   |  
| **ROL**   | `[Rx + const], Ry`     | ` Bitwise rotate left mem[Rx + const] by Ry amount of steps ` | 4 |  
|           |                        |                                                               |   |  
| **ROR**   | `Rx, constant`         | ` Bitwise rotate right Rx by 'constant' amount of steps     ` | 1 |  
| **ROR**   | `Rx, Ry`               | ` Bitwise rotate right Rx by Ry amount of steps             ` | 1 |  
| **ROR**   | `[Rx], Ry`             | ` Bitwise rotate right mem[Rx]  by Ry amount of steps       ` | 3 |  
| **ROR**   | `[Address], Ry`        | ` Bitwise rotate right mem[Address] by Ry amount of steps   ` | 3 |  
| ..3x..    |                        |                                                               |   |  
| **ROR**   | `[Rx + const], Ry`     | ` Bitwise rotate right mem[Rx + const] by Ry amount of steps` | 4 |  
|           |                        |                                                               |   |  
| **CMP**   | `Rx, constant`         | ` Rx - constant        => set N,Z,C,V `                       | 1 |  
| **CMP**   | `Rx, Ry`               | ` Rx - Ry              => set N,Z,C,V `                       | 1 |  
| **CMP**   | `Rx, [Address]`        | ` Rx - mem[Address]    => set N,Z,C,V `                       | 2 |  
| **CMP**   | `Rx, [Ry]`             | ` Rx - mem[Ry]         => set N,Z,C,V `                       | 2 |  
| **CMP**   | `[Address], Rx`        | ` mem[Address] - Rx    => set N,Z,C,V `                       | 2 |  
| **CMP**   | `[Rx], Ry`             | ` mem[Rx] - Ry         => set N,Z,C,V `                       | 2 |  
| **CMP**   | `Rx, [Ry + const]`     | ` Rx - mem[Ry + const] => set N,Z,C,V `                       | 3 |  
| **CMP**   | `[Rx + const], Ry`     | ` mem[Rx + const] - Ry => set N,Z,C,V `                       | 3 |  
|           |                        |                                                               |   |  
| **SEC**   |                        | ` C = 1             `                                         | 1 |  
| **CLC**   |                        | ` C = 0             `                                         | 1 |  
| **SEI**   |                        | ` I = 1             `                                         | 1 |  
| **CLI**   |                        | ` I = 0             `                                         | 1 |  
|           |                        |                                                               |   |  
| **JMP**   | `Address`              | ` PC = Address      `                                         | 1 |  
| **JMP**   | `Rx`                   | ` PC = Rx           `                                         | 1 |  
| **JMP**   | `[Address]`            | ` PC = mem[Address] `                                         | 2 |  
| **JMP**   | `[Rx]`                 | ` PC = mem[Rx]      `                                         | 2 |  
| **JMP**   | `[Rx + const]`         | ` PC = mem[Rx + const]`                                       | 3 |
|           |                        |                                                               |   |
| **JSR**   | `Address`              | ` Push addr. of next instruction to stack, PC = Address      `| 3 |
| **JSR**   | `Rx`                   | ` Push addr. of next instruction to stack, PC = Rx           `| 3 |
| **JSR**   | `[Address]`            | ` Push addr. of next instruction to stack, PC = mem[Address] `| 4 |
| **JSR**   | `[Rx]`                 | ` Push addr. of next instruction to stack, PC = mem[Rx]      `| 4 |
| **JSR**   | `[Rx + const]`         |`Push addr. of next instruction to stack, PC = mem[Rx + const]`| 5 |
| **RTS**   |                        | ` PC = mem[SP++]                                       `      | 3 |  
|           |                        |                                                               |   |  
| **BNE**   | `offset`               | ` if(Z==0) PC += offset `                                     | 1 |  
| **BEQ**   | `offset`               | ` if(Z==1) PC += offset, offset is 32-bit (2nd complement)  ` | 1 |  
| **BCC**   | `offset`               | ` if(C==0) PC += offset `                                     | 1 |  
| **BCS**   | `offset`               | ` if(C==1) PC += offset `                                     | 1 |  
| **BPL**   | `offset`               | ` if(N==0) PC += offset `                                     | 1 |  
| **BMI**   | `offset`               | ` if(N==1) PC += offset `                                     | 1 |  
| **BVC**   | `offset`               | ` if(V==0) PC += offset `                                     | 1 |  
| **BVS**   | `offset`               | ` if(V==1) PC += offset `                                     | 1 |  
| **BGT**   | `offset`               | ` if(greater than) PC += offset `                             | 1 |  
| **BLT**   | `offset`               | ` if(less than) PC += offset `                                | 1 |  
| **BGE**   | `offset`               | ` if(greater or equal) PC += offset `                         | 1 |  
| **BLE**   | `offset`               | ` if(less or equal) PC += offset `                            | 1 |  
| **BRA**   | `offset`               | ` PC += offset, no condition `                                | 1 |  
|           |                        |                                                               |   |
| **PUSH**  | ` constant`            | ` mem[--SP] = constant  `                                     | 2 |  
| **PUSH**  | ` Rx`                  | ` mem[--SP] = Rx;       `                                     | 2 |  
| **PUSH**  | ` [Rx]`                | ` mem[--SP] = mem[Rx]   `                                     | 3 |  
| **PUSH**  | ` [Address]`           | ` mem[--SP] = mem[Address] `                                  | 3 |  
| **PUSH**  | ` [Rx + const]`        | ` mem[--SP] = mem[Rx + const]`                                | 4 |
| **PUSHA** |                        | ` Push R0..R13 onto stack (PHA = PusH All registers) `        | 32 |  
| **POPA**  |                        | ` Pop R0..R13 from stack (POA = POP All registers) `          | 32 |  
|           |                        |                                                               |   |  
| **POP**   | `Rx`                   | ` Rx = mem[SP++]           `                                  | 2 |  
| **POP**   | `[Rx]`                 | ` mem[Rx] = mem[SP++]      `                                  | 3 |  
| **POP**   | `[Address]`            | ` mem[Address] = mem[SP++] `                                  | 3 |  
| **POP**   | `[Rx + const]`         | ` mem[Rx + const] = mem[SP++]      `                          | 4 |  
|           |                        |                                                               |   |  
| **NOP**   |                        | ` no operation             `                                  | 1 |  
| **BRK**   |                        | ` raise software interrupt `                                  | 4 |  
| **RTI**   |                        | ` return from interrupt (restore P, PC) `                     | 4 |  
| **SLP**   |                        | ` Wait until IRQ occurs `                                     | - |
