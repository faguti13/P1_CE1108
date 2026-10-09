; generado por jaf desde tests/golden/listas.oly
; un bundle por linea, slots 0..3 separados por '|'
_inicio:
    SUMI r0, 32, r28
    SUMI r0, 512, r29
    SUMI r0, 10, r8
    DLII r28, 11, r28
    DLII r29, 11, r29
    SUMI r0, 1, r10
    SUMI r28, 0, r9
    SUM r29, r0, r30
    SUMI r29, -64, r29
    SUMI r0, 1, r8 | AP 0(r9), r8
    NOP
    NOP
    SUMI r0, 3, r8 | AP 4(r9), r8
    AP 8(r9), r0
    AP 12(r9), r0
    AP 16(r9), r0
    AP 20(r9), r0
    AP 24(r9), r0
    AP 28(r9), r0
    AP 32(r9), r0
    AP 36(r9), r0
    AP 40(r9), r0
    SUMI r28, 48, r9 | AP 44(r9), r0
    NOP
    NOP
    SUMI r0, 4, r8 | AP 0(r9), r8
    NOP
    NOP
    SUMI r0, 3, r8 | AP 4(r9), r8
    AP 8(r9), r0
    AP 12(r9), r0
    AP 16(r9), r0
    AP 20(r9), r0
    AP 24(r9), r0
    AP 28(r9), r0
    AP 32(r9), r0
    AP 36(r9), r0
    AP 40(r9), r0
    AP 44(r9), r0
    AP 48(r9), r0
    SUMI r28, 104, r9 | AP 52(r9), r0
    NOP
    NOP
    SUMI r0, 1, r8 | AP 0(r9), r8
    NOP
    NOP
    SUMI r0, 2, r8 | AP 4(r9), r8
    SUMI r0, 42, r10 | AP 8(r9), r10
    NOP
    NOP
    SUMI r0, 3, r10 | AP 12(r9), r10
    NOP
    NOP
    SUMI r28, 124, r9 | AP 16(r9), r10
    SUMI r0, 1, r10
    NOP
    SUMI r0, 2, r8 | AP 0(r9), r8
    NOP
    NOP
    AP 4(r9), r8
    SUMI r0, 2, r10 | AP 8(r9), r10
    NOP
    NOP
    SUMI r0, 3, r10 | AP 12(r9), r10
    NOP
    NOP
    SUMI r0, 4, r10 | AP 16(r9), r10
    NOP
    NOP
    AP 20(r9), r10 | SYE f_main
    FIN
f_f:
    SUM r29, r0, r30 | AP -16(r29), r30
    SUMI r29, -88, r29 | AP -12(r29), r31
    SUMI r0, 0, r8
    NOP
    NOP
    SUMI r0, 0, r8 | AP -20(r30), r8
    NOP
    NOP
    AP -24(r30), r8 | S L_0
L_1:
    CP -20(r30), r8
    CP -4(r30), r9
    CP -24(r30), r10
    NOP
    NOP
    DLII r10, 2, r10
    NOP
    NOP
    SUM r9, r10, r9
    NOP
    NOP
    CP 8(r9), r9
    NOP
    NOP
    SUM r8, r9, r8
    NOP
    NOP
    AP -20(r30), r8
    CP -24(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -24(r30), r8
L_0:
    CP -24(r30), r8
    CP -8(r30), r9
    NOP
    NOP
    SMQ r8, r9, L_1
    NOP
    NOP
    CP -20(r30), r8
    NOP
    NOP
    SUM r8, r0, r1
    NOP
    NOP
f_f_fin:
    SUM r30, r0, r29 | CP -12(r30), r31
    CP -16(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30
    SUMI r29, -72, r29 | AP -4(r29), r31
    SUMI r28, 0, r8
    SUMI r0, 0, r9
    SUMI r0, 2, r10
    NOP
    DLII r9, 2, r9
    NOP
    NOP
    SUM r8, r9, r8
    SUMI r0, 5, r9
    NOP
    NOP
    SUMI r28, 48, r8 | AP 8(r8), r9
    SUMI r0, 1, r9
    NOP
    CP 4(r8), r11
    NOP
    NOP
    MUL r9, r11, r9
    NOP
    NOP
    SUM r9, r10, r9
    SUMI r0, 0, r10
    NOP
    DLII r9, 2, r9
    DLII r10, 2, r10
    NOP
    SUM r8, r9, r8
    SUMI r28, 0, r9
    NOP
    NOP
    SUM r9, r10, r9
    SUMI r0, 2, r10
    NOP
    CP 8(r9), r9
    NOP
    NOP
    SUMI r9, 255, r9
    NOP
    NOP
    SUMI r28, 48, r8 | AP 8(r8), r9
    SUMI r0, 1, r9
    NOP
    CP 4(r8), r11
    NOP
    NOP
    MUL r9, r11, r9
    NOP
    NOP
    SUM r9, r10, r9
    SUMI r0, 1, r10
    NOP
    DLII r9, 2, r9
    NOP
    NOP
    SUM r8, r9, r8
    NOP
    NOP
    CP 8(r8), r9
    NOP
    NOP
    REST r9, r10, r9
    NOP
    NOP
    AP 8(r8), r9 | S L_2
L_3:
    FIN
L_2:
    S L_3
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31
    CP -8(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
