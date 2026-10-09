; generado por jaf desde ejercicios/ej3_feistel.oly
; un bundle por linea, slots 0..3 separados por '|'
_inicio:
    SUMI r0, 32, r28
    SUMI r0, 512, r29
    SUMI r0, 8, r8
    DLII r28, 11, r28
    DLII r29, 11, r29
    SUMI r0, 9320, r10
    SUMI r28, 0, r9
    SUM r29, r0, r30
    SUMI r29, -64, r29
    SUMI r0, 1, r8 | AP 0(r9), r8
    DLII r10, 11, r10
    NOP
    SUMI r0, 8, r8 | AP 4(r9), r8
    SUMI r10, 1383, r10
    NOP
    NOP
    SUMI r0, 550, r10 | AP 8(r9), r10
    NOP
    NOP
    DLII r10, 11, r10
    NOP
    NOP
    SUMI r10, 1401, r10
    NOP
    NOP
    DLII r10, 11, r10
    NOP
    NOP
    SUMI r10, 1519, r10
    NOP
    NOP
    SUMI r0, 890, r10 | AP 12(r9), r10
    NOP
    NOP
    DLII r10, 11, r10
    NOP
    NOP
    SUMI r10, 1463, r10
    NOP
    NOP
    DLII r10, 11, r10
    NOP
    NOP
    SUMI r10, 1775, r10
    NOP
    NOP
    SUMI r0, 46, r10 | AP 16(r9), r10
    NOP
    NOP
    DLII r10, 11, r10
    NOP
    NOP
    SUMI r10, 1470, r10
    NOP
    NOP
    DLII r10, 11, r10
    NOP
    NOP
    SUMI r10, 13, r10
    NOP
    NOP
    SUMI r0, 0, r10 | AP 20(r9), r10
    NOP
    NOP
    SUMI r0, 0, r10 | AP 24(r9), r10
    NOP
    NOP
    SUMI r0, -1, r10 | AP 28(r9), r10
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
    SUMI r0, 8, r8 | AP 4(r9), r8
    AP 8(r9), r0
    AP 12(r9), r0
    AP 16(r9), r0
    AP 20(r9), r0
    AP 24(r9), r0
    AP 28(r9), r0
    AP 32(r9), r0
    SUMI r28, 80, r9 | AP 36(r9), r0
    NOP
    NOP
    SUMI r0, 1, r8 | AP 0(r9), r8
    NOP
    NOP
    SUMI r0, 0, r8 | AP 4(r9), r8
    AP 8(r9), r0
    AP 12(r9), r0
    AP 16(r9), r0
    AP 20(r9), r0
    AP 24(r9), r0
    AP 28(r9), r0
    AP 32(r9), r0
    AP 36(r9), r0
    SUMI r0, 0, r8 | AP 120(r28), r8
    NOP
    NOP
    SUMI r0, 0, r8 | AP 124(r28), r8
    NOP
    NOP
    AP 128(r28), r8 | SYE f_main
    FIN
f_configurar_boveda:
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
f_configurar_boveda_fin:
    SUM r30, r0, r29 | CP -4(r30), r31
    CP -8(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_copiar:
    SUM r29, r0, r30 | AP -20(r29), r30
    SUMI r29, -88, r29 | AP -16(r29), r31
    SUMI r0, 0, r8
    NOP
    NOP
    AP -24(r30), r8 | S L_0
L_1:
    CP -8(r30), r8
    CP -24(r30), r9
    NOP
    NOP
    DLII r9, 2, r9
    NOP
    NOP
    SUM r8, r9, r8 | CP -4(r30), r9
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
    AP 8(r8), r9
    CP -24(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -24(r30), r8
L_0:
    CP -24(r30), r8
    CP -12(r30), r9
    NOP
    NOP
    SMQ r8, r9, L_1
    NOP
    NOP
f_copiar_fin:
    SUM r30, r0, r29 | CP -16(r30), r31
    CP -20(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_cifrar_lista:
    SUM r29, r0, r30 | AP -16(r29), r30
    SUMI r29, -92, r29 | AP -12(r29), r31
    SUMI r0, 0, r8
    NOP
    NOP
    AP -20(r30), r8 | S L_2
L_3:
    SUMI r0, 2, r9 | CP -4(r30), r8
    CP -20(r30), r10
    NOP
    NOP
    MUL r9, r10, r9
    NOP
    NOP
    DLII r9, 2, r9
    NOP
    NOP
    SUM r8, r9, r8
    SUMI r0, 2, r9
    NOP
    CP 8(r8), r8
    NOP
    NOP
    AP -24(r30), r8
    CP -4(r30), r8
    CP -20(r30), r10
    NOP
    NOP
    MUL r9, r10, r9
    NOP
    NOP
    SUMI r9, 1, r9
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
    CP -24(r30), r8
    CP -28(r30), r9
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
    AP -24(r30), r8
    SUMI r0, 2, r9 | AP -28(r30), r9
    CP -4(r30), r8
    CP -20(r30), r10
    NOP
    NOP
    MUL r9, r10, r9
    NOP
    NOP
    DLII r9, 2, r9
    NOP
    NOP
    SUM r8, r9, r8 | CP -24(r30), r9
    NOP
    NOP
    SUMI r0, 2, r9 | AP 8(r8), r9
    CP -4(r30), r8
    CP -20(r30), r10
    NOP
    NOP
    MUL r9, r10, r9
    NOP
    NOP
    SUMI r9, 1, r9
    NOP
    NOP
    DLII r9, 2, r9
    NOP
    NOP
    SUM r8, r9, r8 | CP -28(r30), r9
    NOP
    NOP
    AP 8(r8), r9
    CP -20(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -20(r30), r8
L_2:
    CP -20(r30), r8
    CP -8(r30), r9
    NOP
    NOP
    SMQ r8, r9, L_3
    NOP
    NOP
f_cifrar_lista_fin:
    SUM r30, r0, r29 | CP -12(r30), r31
    CP -16(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_descifrar_lista:
    SUM r29, r0, r30 | AP -16(r29), r30
    SUMI r29, -92, r29 | AP -12(r29), r31
    SUMI r0, 0, r8
    NOP
    NOP
    AP -20(r30), r8 | S L_4
L_5:
    SUMI r0, 2, r9 | CP -4(r30), r8
    CP -20(r30), r10
    NOP
    NOP
    MUL r9, r10, r9
    NOP
    NOP
    DLII r9, 2, r9
    NOP
    NOP
    SUM r8, r9, r8
    SUMI r0, 2, r9
    NOP
    CP 8(r8), r8
    NOP
    NOP
    AP -24(r30), r8
    CP -4(r30), r8
    CP -20(r30), r10
    NOP
    NOP
    MUL r9, r10, r9
    NOP
    NOP
    SUMI r9, 1, r9
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
    CP -24(r30), r8
    CP -28(r30), r9
    NOP
    NOP
    FROUND r9, r8, 0, 3, r10, r11
    NOP
    NOP
    FROUND r10, r11, 0, 2, r9, r8
    NOP
    NOP
    FROUND r9, r8, 0, 1, r10, r11
    NOP
    NOP
    FROUND r10, r11, 0, 0, r9, r8
    NOP
    NOP
    AP -24(r30), r8
    SUMI r0, 2, r9 | AP -28(r30), r9
    CP -4(r30), r8
    CP -20(r30), r10
    NOP
    NOP
    MUL r9, r10, r9
    NOP
    NOP
    DLII r9, 2, r9
    NOP
    NOP
    SUM r8, r9, r8 | CP -24(r30), r9
    NOP
    NOP
    SUMI r0, 2, r9 | AP 8(r8), r9
    CP -4(r30), r8
    CP -20(r30), r10
    NOP
    NOP
    MUL r9, r10, r9
    NOP
    NOP
    SUMI r9, 1, r9
    NOP
    NOP
    DLII r9, 2, r9
    NOP
    NOP
    SUM r8, r9, r8 | CP -28(r30), r9
    NOP
    NOP
    AP 8(r8), r9
    CP -20(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -20(r30), r8
L_4:
    CP -20(r30), r8
    CP -8(r30), r9
    NOP
    NOP
    SMQ r8, r9, L_5
    NOP
    NOP
f_descifrar_lista_fin:
    SUM r30, r0, r29 | CP -12(r30), r31
    CP -16(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_iguales:
    SUM r29, r0, r30 | AP -20(r29), r30
    SUMI r29, -88, r29 | AP -16(r29), r31
    SUMI r0, 0, r8
    NOP
    NOP
    AP -24(r30), r8 | S L_6
L_7:
    CP -4(r30), r8
    CP -24(r30), r9
    NOP
    NOP
    DLII r9, 2, r9
    NOP
    NOP
    SUM r8, r9, r8
    NOP
    NOP
    CP 8(r8), r8
    CP -8(r30), r9
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
    SIG r8, r9, L_8
    NOP
    NOP
    SUMI r0, 0, r8
    NOP
    NOP
    SUM r8, r0, r1 | S f_iguales_fin
L_8:
    CP -24(r30), r8
    NOP
    NOP
    SUMI r8, 1, r8
    NOP
    NOP
    AP -24(r30), r8
L_6:
    CP -24(r30), r8
    CP -12(r30), r9
    NOP
    NOP
    SMQ r8, r9, L_7
    NOP
    NOP
    SUMI r0, 1, r8
    NOP
    NOP
    SUM r8, r0, r1
    NOP
    NOP
f_iguales_fin:
    SUM r30, r0, r29 | CP -16(r30), r31
    CP -20(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_suma_de_control:
    SUM r29, r0, r30 | AP -16(r29), r30
    SUMI r29, -88, r29 | AP -12(r29), r31
    SUMI r0, 0, r8
    NOP
    NOP
    SUMI r0, 0, r8 | AP -20(r30), r8
    NOP
    NOP
    AP -24(r30), r8 | S L_9
L_10:
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
L_9:
    CP -24(r30), r8
    CP -8(r30), r9
    NOP
    NOP
    SMQ r8, r9, L_10
    NOP
    NOP
    CP -20(r30), r8
    NOP
    NOP
    SUM r8, r0, r1
    NOP
    NOP
f_suma_de_control_fin:
    SUM r30, r0, r29 | CP -12(r30), r31
    CP -16(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30
    SUMI r29, -72, r29 | AP -4(r29), r31 | SYE f_configurar_boveda
    SUMI r28, 0, r8
    SUMI r28, 40, r9
    SUMI r0, 8, r10
    AP -4(r29), r8
    AP -8(r29), r9
    AP -12(r29), r10 | SYE f_copiar
    SUMI r28, 0, r8
    SUMI r0, 4, r9
    NOP
    AP -4(r29), r8
    AP -8(r29), r9 | SYE f_cifrar_lista
    SUMI r28, 0, r8
    SUMI r28, 80, r9
    SUMI r0, 8, r10
    AP -4(r29), r8
    AP -8(r29), r9
    AP -12(r29), r10 | SYE f_copiar
    SUMI r28, 80, r8
    SUMI r0, 8, r9
    NOP
    AP -4(r29), r8
    AP -8(r29), r9 | SYE f_suma_de_control
    SUM r1, r0, r8
    SUMI r0, 4, r9
    NOP
    SUMI r28, 0, r8 | AP 124(r28), r8
    NOP
    NOP
    AP -4(r29), r8
    AP -8(r29), r9 | SYE f_descifrar_lista
    SUMI r28, 0, r8
    SUMI r28, 40, r9
    SUMI r0, 8, r10
    AP -4(r29), r8
    AP -8(r29), r9
    AP -12(r29), r10 | SYE f_iguales
    SUM r1, r0, r8
    NOP
    NOP
    AP 120(r28), r8 | RDSR r8
    LOGOUT
    NOP
    AP 128(r28), r8
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31
    CP -8(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
