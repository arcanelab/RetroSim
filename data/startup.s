.pc = $14
.dword InstructionExceptionHandler

.pc = $200

.def PRINT = $face

start:
    sys PRINT, $12345678

loop:
    jmp loop

InstructionExceptionHandler:

    slp
    jmp InstructionExceptionHandler
