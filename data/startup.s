.pc = $200

start:
    sys $8412, $12345678
loop:
    jmp loop
