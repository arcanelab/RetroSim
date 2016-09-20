        .pc = $c000

        clr     r0
loop:
        mov.w   [$2006], r0
        inc     r0
        cmp.w   r0, 680
        bne     loop
        clr     r0
        jmp     loop
