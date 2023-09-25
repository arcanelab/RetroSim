# A65000 Assembler

## General

You can write your assembly code either in lower, upper or mixed case, however, during assembly everything will be converted to lower case, meaning that labels and symbols will be treated in a case insensitive fashion.

## Numbers

Three different numeral systems are supported:

*  decimal: a number consisting only of numerical characters,
*  hexadecimal: a $ character followed by a hexadecimal number
*  binary: a % character followed by a series of 1's and 0's.

The upper limit for numerical values is 32 bits. Should the assembler encounter a number bigger than `$FFFFFFFF`, it will present you with a "_value out of range_" error message.

## Symbols

A symbol is a string associated with a numerical value. During the assembly process, all symbols will be substituted with the numerical values associated with them. Symbols, just as everything else in the assembly file, will be handled as case-insensitive entities.

Symbols can be created in two ways. Via:

*  labels,
*  using the `.def` directive.

Symbols must begin with a letter, followed by any number of alphanumeric characters, including the underscore character.

Examples:

*  Valid symbols are: `back5, Names, color_, Big_Cats, PI`
*  Invalid symbols are: `_loop, 34dogs, !code, _left_`

A label can be defined at the beginning of a line (preceding white space is allowed), given with a unique name followed by the colon character. Such a label definition will create a symbol with the given name, assiociated with the value of the current memory address it is assembled at.

The other way to create a symbol is by using the `.def` directive. The syntax is one of the following:

*  `.def <name> = <number>`
*  `.def <name> = <symbol> + <number>`

The latter is useful for defining register areas by using a base address and defining the rest of the addresses as offsets to it.

Examples:

Valid symbol definitions:

    .def GPUBASE = $8f005000
    .def GFXMode = GPUBASE + 1
    .def GPUReg2 = GPUBASE + 2
    .def Magic_Number = $1234567
    .def binaryCode = %11001010

Invalid symbol definitions:

    .def _REG = $f000            ; symbol must start with a letter
    .def ProcessID = $1000 + 3   ; adding two numbers is not allowed, the left side of the plus character must be an already defined symbol.
    .def HUGEnumber = $123456789 ; defined value is too big to be stored in 32 bits.

Label references and symbols defined by the `.def` directive are interchangeable. That means wherever you can write a label reference (a reference being any appearance of the label except for its definition) you can also write a symbol defined by a `.def` directive and vice versa.

## Comments

Everything between a `;` character and the end of the line is a comment and is discarded by the assembler.

## Directives

Directives are special keywords understood by the assembler. Common to all directives is that they are specified by a period character followed by the keyword.

There's a directive that every assembly program must contain:

### The .PC Directive

Syntax:

    .pc = <address>

This keyword specifies the starting memory address for the assembly procedure. You can use it multiple times in an assembly file.

Example:

    .pc = $1000

    clr r0
    mov r1, [name + r1]
    rol r1, 2
    mov [name + r1], r1

    .pc = $2000

    name: .text "hey"
