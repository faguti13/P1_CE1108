; generado por jaf desde tests/golden/minimo.oly
; un bundle por linea, slots 0..3 separados por '|'
_inicio:
    SUMI r0, 32, r28
    SUMI r0, 512, r29
    SUMI r0, 7, r8
    DLII r28, 11, r28
    DLII r29, 11, r29
    NOP
    AP 0(r28), r8
    SUM r29, r0, r30
    SUMI r29, -64, r29 | SYE f_main
    FIN
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30
    SUMI r29, -72, r29 | AP -4(r29), r31
    CP 0(r28), r8
    NOP
    NOP
    MULI r8, 6, r8
    NOP
    NOP
    AP 0(r28), r8
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31
    CP -8(r30), r27
    NOP
    NOP
    SUM r27, r0, r30 | SRG r31
