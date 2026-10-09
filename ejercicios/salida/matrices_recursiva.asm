; generado por jaf desde ejercicios/matrices_recursiva.oly
; un bundle por linea, slots 0..3 separados por '|'
_inicio:
    SUMI r0, 32, r28
    SUMI r0, 512, r29
    SUMI r0, 2, r8
    DLII r28, 11, r28
    DLII r29, 11, r29
    SUMI r0, 1, r10
    SUMI r28, 0, r9
    SUM r29, r0, r30
    SUMI r29, -64, r29
    SUMI r0, 3, r8 | AP 0(r9), r8
    NOP
    NOP
    SUMI r0, 0, r8 | AP 4(r9), r8
    SUMI r0, 2, r10 | AP 8(r9), r10
    NOP
    NOP
    SUMI r0, 3, r10 | AP 12(r9), r10
    NOP
    NOP
    SUMI r0, 4, r10 | AP 16(r9), r10
    NOP
    NOP
    SUMI r0, 5, r10 | AP 20(r9), r10
    NOP
    NOP
    SUMI r0, 6, r10 | AP 24(r9), r10
    NOP
    NOP
    AP 28(r9), r10
    SUMI r0, 0, r8 | AP 32(r28), r8
    NOP
    NOP
    SUMI r0, 0, r8 | AP 36(r28), r8
    NOP
    NOP
    AP 40(r28), r8 | SYE f_main
    FIN
f_suma_fila:
    SUM r29, r0, r30 | AP -24(r29), r30
    SUMI r29, -88, r29 | AP -20(r29), r31
    NOP
    CP -12(r30), r8
    CP -16(r30), r9
    NOP
    NOP
    SMQ r8, r9, L_0
    NOP
    NOP
    SUMI r0, 0, r8
    NOP
    NOP
    SUM r8, r0, r1 | S f_suma_fila_fin
L_0:
    CP -4(r30), r8
    CP -8(r30), r9
    CP -12(r30), r10
    CP 4(r8), r11
    NOP
    NOP
    MUL r9, r11, r9
    NOP
    NOP
    SUM r9, r10, r9
    NOP
    NOP
    DLII r9, 2, r9
    NOP
    NOP
    SUM r8, r9, r8
    NOP
    NOP
    CP 8(r8), r8
    NOP
    NOP
    AP -28(r30), r8
    CP -4(r30), r9
    CP -8(r30), r10
    CP -12(r30), r11
    CP -16(r30), r12
    AP -4(r29), r9
    SUMI r11, 1, r11 | AP -8(r29), r10
    NOP
    NOP
    AP -12(r29), r11
    AP -16(r29), r12 | SYE f_suma_fila
    SUM r1, r0, r9 | CP -28(r30), r8
    NOP
    NOP
    SUM r8, r9, r8
    NOP
    NOP
    SUM r8, r0, r1
    NOP
    NOP
f_suma_fila_fin:
    SUM r30, r0, r29 | CP -20(r30), r31
    CP -24(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_suma_matriz:
    SUM r29, r0, r30 | AP -24(r29), r30
    SUMI r29, -88, r29 | AP -20(r29), r31
    NOP
    CP -8(r30), r8
    CP -12(r30), r9
    NOP
    NOP
    SMQ r8, r9, L_1
    NOP
    NOP
    SUMI r0, 0, r8
    NOP
    NOP
    SUM r8, r0, r1 | S f_suma_matriz_fin
L_1:
    SUMI r0, 0, r10 | CP -4(r30), r8
    CP -8(r30), r9
    CP -16(r30), r11
    AP -4(r29), r8
    AP -8(r29), r9
    AP -12(r29), r10
    AP -16(r29), r11 | SYE f_suma_fila
    SUM r1, r0, r8
    NOP
    NOP
    AP -28(r30), r8
    CP -4(r30), r9
    CP -8(r30), r10
    CP -12(r30), r11
    CP -16(r30), r12
    SUMI r10, 1, r10 | AP -4(r29), r9
    NOP
    NOP
    AP -8(r29), r10
    AP -12(r29), r11
    AP -16(r29), r12 | SYE f_suma_matriz
    SUM r1, r0, r9 | CP -28(r30), r8
    NOP
    NOP
    SUM r8, r9, r8
    NOP
    NOP
    SUM r8, r0, r1
    NOP
    NOP
f_suma_matriz_fin:
    SUM r30, r0, r29 | CP -20(r30), r31
    CP -24(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_fib:
    SUM r29, r0, r30 | AP -12(r29), r30
    SUMI r29, -76, r29 | AP -8(r29), r31
    SUMI r0, 2, r9
    CP -4(r30), r8
    NOP
    NOP
    SMI r8, r9, L_2
    NOP
    NOP
    CP -4(r30), r8
    NOP
    NOP
    SUM r8, r0, r1 | S f_fib_fin
L_2:
    CP -4(r30), r8
    NOP
    NOP
    RESTI r8, 1, r8
    NOP
    NOP
    AP -4(r29), r8 | SYE f_fib
    SUM r1, r0, r8
    NOP
    NOP
    AP -16(r30), r8
    CP -4(r30), r9
    NOP
    NOP
    RESTI r9, 2, r9
    NOP
    NOP
    AP -4(r29), r9 | SYE f_fib
    SUM r1, r0, r9 | CP -16(r30), r8
    NOP
    NOP
    SUM r8, r9, r8
    NOP
    NOP
    SUM r8, r0, r1
    NOP
    NOP
f_fib_fin:
    SUM r30, r0, r29 | CP -8(r30), r31
    CP -12(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_fact:
    SUM r29, r0, r30 | AP -12(r29), r30
    SUMI r29, -76, r29 | AP -8(r29), r31
    SUMI r0, 1, r9
    CP -4(r30), r8
    NOP
    NOP
    SMQ r9, r8, L_3
    NOP
    NOP
    SUMI r0, 1, r8
    NOP
    NOP
    SUM r8, r0, r1 | S f_fact_fin
L_3:
    CP -4(r30), r8
    NOP
    NOP
    AP -16(r30), r8
    CP -4(r30), r9
    NOP
    NOP
    RESTI r9, 1, r9
    NOP
    NOP
    AP -4(r29), r9 | SYE f_fact
    SUM r1, r0, r9 | CP -16(r30), r8
    NOP
    NOP
    MUL r8, r9, r8
    NOP
    NOP
    SUM r8, r0, r1
    NOP
    NOP
f_fact_fin:
    SUM r30, r0, r29 | CP -8(r30), r31
    CP -12(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30
    SUMI r29, -72, r29 | AP -4(r29), r31
    SUMI r28, 0, r8
    SUMI r0, 0, r9
    SUMI r0, 2, r10
    SUMI r0, 3, r11 | AP -4(r29), r8
    AP -8(r29), r9
    AP -12(r29), r10
    AP -16(r29), r11 | SYE f_suma_matriz
    SUM r1, r0, r8
    NOP
    NOP
    SUMI r0, 10, r8 | AP 32(r28), r8
    NOP
    NOP
    AP -4(r29), r8 | SYE f_fib
    SUM r1, r0, r8
    NOP
    NOP
    SUMI r0, 6, r8 | AP 36(r28), r8
    NOP
    NOP
    AP -4(r29), r8 | SYE f_fact
    SUM r1, r0, r8
    NOP
    NOP
    AP 40(r28), r8
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31
    CP -8(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
