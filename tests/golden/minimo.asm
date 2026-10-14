; generado por jaf desde tests/golden/minimo.oly
; un bundle por linea, slots 0..3 separados por '|'
; @xxxxxxxx es el byte del bundle en el .bin (cada bundle ocupa 16)
; oly:N es la linea del fuente que produjo la instruccion
_inicio:
    SUMI r0, 32, r28                                                 ; @00000000  oly:1
    SUMI r0, 512, r29                                                ; @00000010  oly:1
    SUMI r0, 7, r8                                                   ; @00000020  oly:2
    DLII r28, 11, r28                                                ; @00000030  oly:1
    DLII r29, 11, r29                                                ; @00000040  oly:1
    NOP                                                              ; @00000050
    AP 0(r28), r8                                                    ; @00000060  oly:2
    SUM r29, r0, r30                                                 ; @00000070  oly:1
    SUMI r29, -64, r29 | SYE f_main                                  ; @00000080  oly:1,2
    FIN                                                              ; @00000090  oly:2
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @000000a0  oly:3
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @000000b0  oly:3
    CP 0(r28), r8                                                    ; @000000c0  oly:4
    NOP                                                              ; @000000d0
    NOP                                                              ; @000000e0
    MULI r8, 6, r8                                                   ; @000000f0  oly:4
    NOP                                                              ; @00000100
    NOP                                                              ; @00000110
    AP 0(r28), r8                                                    ; @00000120  oly:4
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000130  oly:4
    CP -8(r30), r27                                                  ; @00000140  oly:4
    NOP                                                              ; @00000150
    NOP                                                              ; @00000160
    SUM r27, r0, r30 | SRG r31                                       ; @00000170  oly:4
