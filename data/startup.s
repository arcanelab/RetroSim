.def PRINT = $face

.pc = $14

.dword InstructionExceptionHandler

.pc = $200

start:
    sys PRINT, $12345678

loop:
    bra loop

InstructionExceptionHandler:

    slp
    bra InstructionExceptionHandler
