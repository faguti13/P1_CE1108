; generado por jaf desde tests/golden/factorial.oly
; un bundle por linea, slots 0..3 separados por '|'
_inicio:
    SUMI r0, 32, r28
    SUMI r0, 512, r29
    SUMI r0, 0, r8
    DLII r28, 11, r28
    DLII r29, 11, r29
    NOP
    SUMI r0, 0, r8 | AP 0(r28), r8
    SUM r29, r0, r30
    SUMI r29, -64, r29
    AP 4(r28), r8 | SYE f_main
    FIN
f_factorial:
    SUM r29, r0, r30 | AP -12(r29), r30
    SUMI r29, -84, r29 | AP -8(r29), r31
    SUMI r0, 1, r8
    NOP
    NOP
    AP -16(r30), r8
    CP -4(r30), r8
    NOP
    NOP
    SMI r8, r0, L_0
    NOP
    NOP
    SUMI r0, 1, r8
    NOP
    NOP
    REST r0, r8, r8
    NOP
    NOP
    SUM r8, r0, r1 | S f_factorial_fin
L_0:
    SUMI r0, 2, r8
    NOP
    NOP
    AP -20(r30), r8 | S L_1
L_2:
    CP -16(r30), r8
    CP -20(r30), r9
    NOP
    NOP
    MUL r8, r9, r8
    NOP
    NOP
    AP -16(r30), r8
    CP -20(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -20(r30), r8
L_1:
    CP -20(r30), r8
    CP -4(r30), r9
    NOP
    NOP
    SMI r9, r8, L_2
    NOP
    NOP
    CP -16(r30), r8
    NOP
    NOP
    SUM r8, r0, r1
    NOP
    NOP
f_factorial_fin:
    SUM r30, r0, r29 | CP -8(r30), r31
    CP -12(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30
    SUMI r29, -72, r29 | AP -4(r29), r31
    SUMI r0, 5, r8
    NOP
    NOP
    AP -4(r29), r8 | SYE f_factorial
    SUM r1, r0, r8
    NOP
    NOP
    SUMI r0, 3, r8 | AP 0(r28), r8
    NOP
    NOP
    REST r0, r8, r8
    NOP
    NOP
    AP -4(r29), r8 | SYE f_factorial
    SUM r1, r0, r8
    NOP
    NOP
    AP 4(r28), r8
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31
    CP -8(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
