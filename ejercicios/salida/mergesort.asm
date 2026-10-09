; generado por jaf desde ejercicios/mergesort.oly
; un bundle por linea, slots 0..3 separados por '|'
_inicio:
    SUMI r0, 32, r28
    SUMI r0, 512, r29
    SUMI r0, 8, r8
    DLII r28, 11, r28
    DLII r29, 11, r29
    SUMI r0, 38, r10
    SUMI r28, 0, r9
    SUM r29, r0, r30
    SUMI r29, -64, r29
    SUMI r0, 1, r8 | AP 0(r9), r8
    NOP
    NOP
    SUMI r0, 8, r8 | AP 4(r9), r8
    SUMI r0, 27, r10 | AP 8(r9), r10
    NOP
    NOP
    SUMI r0, 43, r10 | AP 12(r9), r10
    NOP
    NOP
    SUMI r0, 3, r10 | AP 16(r9), r10
    NOP
    NOP
    SUMI r0, 9, r10 | AP 20(r9), r10
    NOP
    NOP
    SUMI r0, 82, r10 | AP 24(r9), r10
    NOP
    NOP
    SUMI r0, 10, r10 | AP 28(r9), r10
    NOP
    NOP
    SUMI r0, 1, r10 | AP 32(r9), r10
    NOP
    NOP
    SUMI r28, 40, r9 | AP 36(r9), r10
    NOP
    NOP
    SUMI r0, 1, r8 | AP 0(r9), r8
    NOP
    NOP
    AP 4(r9), r8
    AP 8(r9), r0
    AP 12(r9), r0
    AP 16(r9), r0
    AP 20(r9), r0
    AP 24(r9), r0
    AP 28(r9), r0
    AP 32(r9), r0
    AP 36(r9), r0 | SYE f_main
    FIN
f_fusionar:
    SUM r29, r0, r30 | AP -20(r29), r30
    SUMI r29, -100, r29 | AP -16(r29), r31
    NOP
    CP -4(r30), r8
    NOP
    NOP
    AP -24(r30), r8
    CP -8(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -28(r30), r8
    CP -4(r30), r8
    NOP
    NOP
    AP -32(r30), r8 | S L_0
L_1:
    SUMI r28, 0, r8 | CP -24(r30), r9
    NOP
    NOP
    DLII r9, 2, r9
    NOP
    NOP
    SUM r8, r9, r8
    SUMI r28, 0, r9
    NOP
    CP 8(r8), r8
    CP -28(r30), r10
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
    SMQ r9, r8, L_2
    NOP
    NOP
    SUMI r28, 40, r8 | CP -32(r30), r9
    CP -24(r30), r10
    NOP
    DLII r9, 2, r9
    DLII r10, 2, r10
    NOP
    SUM r8, r9, r8
    SUMI r28, 0, r9
    NOP
    NOP
    SUM r9, r10, r9
    NOP
    NOP
    CP 8(r9), r9
    NOP
    NOP
    AP 8(r8), r9
    CP -24(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -24(r30), r8 | S L_3
L_2:
    SUMI r28, 40, r8 | CP -32(r30), r9
    CP -28(r30), r10
    NOP
    DLII r9, 2, r9
    DLII r10, 2, r10
    NOP
    SUM r8, r9, r8
    SUMI r28, 0, r9
    NOP
    NOP
    SUM r9, r10, r9
    NOP
    NOP
    CP 8(r9), r9
    NOP
    NOP
    AP 8(r8), r9
    CP -28(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -28(r30), r8
L_3:
    CP -32(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -32(r30), r8
L_0:
    CP -24(r30), r8
    CP -8(r30), r9
    NOP
    NOP
    SMQ r9, r8, L_4
    NOP
    NOP
    CP -28(r30), r8
    CP -12(r30), r9
    NOP
    NOP
    SMI r9, r8, L_1
    NOP
    NOP
L_4:
    S L_5
L_6:
    SUMI r28, 40, r8 | CP -32(r30), r9
    CP -24(r30), r10
    NOP
    DLII r9, 2, r9
    DLII r10, 2, r10
    NOP
    SUM r8, r9, r8
    SUMI r28, 0, r9
    NOP
    NOP
    SUM r9, r10, r9
    NOP
    NOP
    CP 8(r9), r9
    NOP
    NOP
    AP 8(r8), r9
    CP -24(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -24(r30), r8
    CP -32(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -32(r30), r8
L_5:
    CP -24(r30), r8
    CP -8(r30), r9
    NOP
    NOP
    SMI r9, r8, L_6
    NOP
    NOP
    S L_7
L_8:
    SUMI r28, 40, r8 | CP -32(r30), r9
    CP -28(r30), r10
    NOP
    DLII r9, 2, r9
    DLII r10, 2, r10
    NOP
    SUM r8, r9, r8
    SUMI r28, 0, r9
    NOP
    NOP
    SUM r9, r10, r9
    NOP
    NOP
    CP 8(r9), r9
    NOP
    NOP
    AP 8(r8), r9
    CP -28(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -28(r30), r8
    CP -32(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -32(r30), r8
L_7:
    CP -28(r30), r8
    CP -12(r30), r9
    NOP
    NOP
    SMI r9, r8, L_8
    NOP
    NOP
    CP -4(r30), r8
    NOP
    NOP
    AP -36(r30), r8 | S L_9
L_10:
    SUMI r28, 0, r8 | CP -36(r30), r9
    CP -36(r30), r10
    NOP
    DLII r9, 2, r9
    DLII r10, 2, r10
    NOP
    SUM r8, r9, r8
    SUMI r28, 40, r9
    NOP
    NOP
    SUM r9, r10, r9
    NOP
    NOP
    CP 8(r9), r9
    NOP
    NOP
    AP 8(r8), r9
    CP -36(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -36(r30), r8
L_9:
    CP -36(r30), r8
    CP -12(r30), r9
    NOP
    NOP
    SMI r9, r8, L_10
    NOP
    NOP
f_fusionar_fin:
    SUM r30, r0, r29 | CP -16(r30), r31
    CP -20(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_ordenar:
    SUM r29, r0, r30 | AP -16(r29), r30
    SUMI r29, -84, r29 | AP -12(r29), r31
    NOP
    CP -4(r30), r8
    CP -8(r30), r9
    NOP
    NOP
    SMI r8, r9, L_11
    NOP
    NOP
    CP -4(r30), r8
    CP -8(r30), r9
    NOP
    NOP
    SUM r8, r9, r8
    NOP
    NOP
    DIVI r8, 2, r8
    NOP
    NOP
    AP -20(r30), r8
    CP -4(r30), r8
    CP -20(r30), r9
    NOP
    AP -4(r29), r8
    AP -8(r29), r9 | SYE f_ordenar
    CP -20(r30), r8
    CP -8(r30), r9
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -4(r29), r8
    AP -8(r29), r9 | SYE f_ordenar
    CP -4(r30), r8
    CP -20(r30), r9
    CP -8(r30), r10
    AP -4(r29), r8
    AP -8(r29), r9
    AP -12(r29), r10 | SYE f_fusionar
L_11:
f_ordenar_fin:
    SUM r30, r0, r29 | CP -12(r30), r31
    CP -16(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30
    SUMI r29, -72, r29 | AP -4(r29), r31
    SUMI r0, 0, r8
    SUMI r0, 7, r9
    NOP
    AP -4(r29), r8
    AP -8(r29), r9 | SYE f_ordenar
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31
    CP -8(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
