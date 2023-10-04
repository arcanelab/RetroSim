.def PRINT = $face

.pc = $14

.dword InstructionExceptionHandler

.pc = $1abe

future:
    bra future

.pc = $200

start:
    sys PRINT, $12345678
    nop
    bne next
    mov r0, 1
    mov.w r0, 1
    mov.b r0, 1
    push.w $1234
;    mov [r0], 1
;    mov.b [r0], 1
;    mov.w [r0], 1

next:
    nop
    clr r0

loop:
    nop
    bra future

InstructionExceptionHandler:

    slp
    bra InstructionExceptionHandler
