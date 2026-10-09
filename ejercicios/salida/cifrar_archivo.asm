; generado por jaf desde ejercicios/cifrar_archivo.oly
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
f_preparar:
    SUM r29, r0, r30 | AP -8(r29), r30
    SUMI r29, -72, r29 | AP -4(r29), r31
    SUMI r0, 811, r8
    NOP
    NOP
    DLII r8, 11, r8
    NOP
    NOP
    SUMI r8, 1986, r8
    NOP
    NOP
    DLII r8, 11, r8
    NOP
    NOP
    SUMI r8, 564, r8
    NOP
    NOP
    SUMI r0, 548, r8 | AUTH r8
    NOP
    NOP
    DLII r8, 11, r8
    NOP
    NOP
    SUMI r8, 563, r8
    NOP
    NOP
    SUM r0, r0, r8 | LOADKEY r8, 0, 0
    SUMI r0, 273, r8
    NOP
    NOP
    DLII r8, 11, r8
    NOP
    NOP
    SUMI r8, 684, r8
    NOP
    NOP
    DLII r8, 11, r8
    NOP
    NOP
    SUMI r8, 1655, r8
    NOP
    NOP
    SUM r0, r0, r8 | LOADKEY r8, 0, 1
    SUMI r0, 546, r8
    NOP
    NOP
    DLII r8, 11, r8
    NOP
    NOP
    SUMI r8, 821, r8
    NOP
    NOP
    DLII r8, 11, r8
    NOP
    NOP
    SUMI r8, 699, r8
    NOP
    NOP
    SUM r0, r0, r8 | LOADKEY r8, 0, 2
    SUMI r0, 819, r8
    NOP
    NOP
    DLII r8, 11, r8
    NOP
    NOP
    SUMI r8, 957, r8
    NOP
    NOP
    DLII r8, 11, r8
    NOP
    NOP
    SUMI r8, 1791, r8
    NOP
    NOP
    SUM r0, r0, r8 | LOADKEY r8, 0, 3
    NOP
    NOP
f_preparar_fin:
    SUM r30, r0, r29 | CP -4(r30), r31
    CP -8(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30
    SUMI r29, -84, r29 | AP -4(r29), r31
    SUMI r0, 0, r8
    NOP
    NOP
    SUMI r0, 0, r8 | AP -12(r30), r8
    NOP
    NOP
    AP -16(r30), r8 | SYE f_preparar
    SUMI r0, 63, r8
    NOP
    NOP
    DLII r8, 11, r8
    NOP
    NOP
    SUMI r8, 2032, r8
    NOP
    NOP
    CP 0(r8), r8
    NOP
    NOP
    SUMI r0, 63, r8 | AP 0(r28), r8
    NOP
    NOP
    DLII r8, 11, r8
    NOP
    NOP
    SUMI r8, 2040, r8
    NOP
    NOP
    CP 0(r8), r8
    NOP
    NOP
    SUMI r0, 0, r8 | AP 4(r28), r8
    NOP
    NOP
    AP -20(r30), r8 | S L_0
L_1:
    SUMI r0, 8, r9 | CP 0(r28), r8
    CP -20(r30), r10
    NOP
    NOP
    MUL r9, r10, r9
    NOP
    NOP
    SUM r8, r9, r8
    SUMI r0, 8, r9
    NOP
    CP 0(r8), r8
    NOP
    NOP
    AP -12(r30), r8
    CP 0(r28), r8
    CP -20(r30), r10
    NOP
    NOP
    MUL r9, r10, r9
    NOP
    NOP
    SUM r8, r9, r8
    NOP
    NOP
    SUMI r8, 4, r8
    NOP
    NOP
    CP 0(r8), r8
    NOP
    NOP
    AP -16(r30), r8
    CP -12(r30), r8
    CP -16(r30), r9
    NOP
    NOP
    FROUND r8, r9, 0, 0, r10, r11
    NOP
    NOP
    FROUND r10, r11, 0, 1, r8, r9
    NOP
    NOP
    FROUND r8, r9, 0, 2, r10, r11
    NOP
    NOP
    FROUND r10, r11, 0, 3, r8, r9
    NOP
    NOP
    AP -12(r30), r8
    SUMI r0, 8, r9 | AP -16(r30), r9
    CP 0(r28), r8
    CP -20(r30), r10
    NOP
    NOP
    MUL r9, r10, r9
    NOP
    NOP
    SUM r8, r9, r8 | CP -12(r30), r9
    NOP
    NOP
    SUMI r0, 8, r9 | AP 0(r8), r9
    CP 0(r28), r8
    CP -20(r30), r10
    NOP
    NOP
    MUL r9, r10, r9
    NOP
    NOP
    SUM r8, r9, r8 | CP -16(r30), r9
    NOP
    NOP
    SUMI r8, 4, r8
    NOP
    NOP
    AP 0(r8), r9
    CP -20(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -20(r30), r8
L_0:
    CP -20(r30), r8
    CP 4(r28), r9
    NOP
    NOP
    SMQ r8, r9, L_1
    NOP
    NOP
    LOGOUT
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31
    CP -8(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
