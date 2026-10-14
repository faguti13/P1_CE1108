; generado por jaf desde tests/golden/factorial.oly
; un bundle por linea, slots 0..3 separados por '|'
; @xxxxxxxx es el byte del bundle en el .bin (cada bundle ocupa 16)
; oly:N es la linea del fuente que produjo la instruccion
_inicio:
    SUMI r0, 32, r28                                                 ; @00000000  oly:1
    SUMI r0, 512, r29                                                ; @00000010  oly:1
    SUMI r0, 0, r8                                                   ; @00000020  oly:5
    DLII r28, 11, r28                                                ; @00000030  oly:1
    DLII r29, 11, r29                                                ; @00000040  oly:1
    NOP                                                              ; @00000050
    SUMI r0, 0, r8 | AP 0(r28), r8                                   ; @00000060  oly:6,5
    SUM r29, r0, r30                                                 ; @00000070  oly:1
    SUMI r29, -64, r29                                               ; @00000080  oly:1
    AP 4(r28), r8 | SYE f_main                                       ; @00000090  oly:6
    FIN                                                              ; @000000a0  oly:6
f_factorial:
    SUM r29, r0, r30 | AP -12(r29), r30                              ; @000000b0  oly:8
    SUMI r29, -84, r29 | AP -8(r29), r31                             ; @000000c0  oly:8
    SUMI r0, 1, r8                                                   ; @000000d0  oly:9
    NOP                                                              ; @000000e0
    NOP                                                              ; @000000f0
    AP -16(r30), r8                                                  ; @00000100  oly:9
    CP -4(r30), r8                                                   ; @00000110  oly:10
    NOP                                                              ; @00000120
    NOP                                                              ; @00000130
    SMI r8, r0, L_0                                                  ; @00000140  oly:10
    NOP                                                              ; @00000150
    NOP                                                              ; @00000160
    SUMI r0, 1, r8                                                   ; @00000170  oly:11
    NOP                                                              ; @00000180
    NOP                                                              ; @00000190
    REST r0, r8, r8                                                  ; @000001a0  oly:11
    NOP                                                              ; @000001b0
    NOP                                                              ; @000001c0
    SUM r8, r0, r1 | S f_factorial_fin                               ; @000001d0  oly:11
L_0:
    SUMI r0, 2, r8                                                   ; @000001e0  oly:13
    NOP                                                              ; @000001f0
    NOP                                                              ; @00000200
    AP -20(r30), r8 | S L_1                                          ; @00000210  oly:13
L_2:
    CP -16(r30), r8                                                  ; @00000220  oly:14
    CP -20(r30), r9                                                  ; @00000230  oly:14
    NOP                                                              ; @00000240
    NOP                                                              ; @00000250
    MUL r8, r9, r8                                                   ; @00000260  oly:14
    NOP                                                              ; @00000270
    NOP                                                              ; @00000280
    AP -16(r30), r8                                                  ; @00000290  oly:14
    CP -20(r30), r8                                                  ; @000002a0  oly:13
    NOP                                                              ; @000002b0
    NOP                                                              ; @000002c0
    SUMI r8, 1, r8                                                   ; @000002d0  oly:13
    NOP                                                              ; @000002e0
    NOP                                                              ; @000002f0
    AP -20(r30), r8                                                  ; @00000300  oly:13
L_1:
    CP -20(r30), r8                                                  ; @00000310  oly:13
    CP -4(r30), r9                                                   ; @00000320  oly:13
    NOP                                                              ; @00000330
    NOP                                                              ; @00000340
    SMI r9, r8, L_2                                                  ; @00000350  oly:13
    NOP                                                              ; @00000360
    NOP                                                              ; @00000370
    CP -16(r30), r8                                                  ; @00000380  oly:16
    NOP                                                              ; @00000390
    NOP                                                              ; @000003a0
    SUM r8, r0, r1                                                   ; @000003b0  oly:16
    NOP                                                              ; @000003c0
    NOP                                                              ; @000003d0
f_factorial_fin:
    SUM r30, r0, r29 | CP -8(r30), r31                               ; @000003e0  oly:16
    CP -12(r30), r27                                                 ; @000003f0  oly:16
    NOP                                                              ; @00000400
    NOP                                                              ; @00000410
    SUM r27, r0, r30 | SRG r31                                       ; @00000420  oly:16
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @00000430  oly:19
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @00000440  oly:19
    SUMI r0, 5, r8                                                   ; @00000450  oly:20
    NOP                                                              ; @00000460
    NOP                                                              ; @00000470
    AP -4(r29), r8 | SYE f_factorial                                 ; @00000480  oly:20
    SUM r1, r0, r8                                                   ; @00000490  oly:20
    NOP                                                              ; @000004a0
    NOP                                                              ; @000004b0
    SUMI r0, 3, r8 | AP 0(r28), r8                                   ; @000004c0  oly:21,20
    NOP                                                              ; @000004d0
    NOP                                                              ; @000004e0
    REST r0, r8, r8                                                  ; @000004f0  oly:21
    NOP                                                              ; @00000500
    NOP                                                              ; @00000510
    AP -4(r29), r8 | SYE f_factorial                                 ; @00000520  oly:21
    SUM r1, r0, r8                                                   ; @00000530  oly:21
    NOP                                                              ; @00000540
    NOP                                                              ; @00000550
    AP 4(r28), r8                                                    ; @00000560  oly:21
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000570  oly:21
    CP -8(r30), r27                                                  ; @00000580  oly:21
    NOP                                                              ; @00000590
    NOP                                                              ; @000005a0
    SUM r27, r0, r30 | SRG r31                                       ; @000005b0  oly:21
