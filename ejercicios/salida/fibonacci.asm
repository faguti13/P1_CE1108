; generado por jaf desde ejercicios/fibonacci.oly
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
f_fibonacci:
    SUM r29, r0, r30 | AP -12(r29), r30                              ; @000000b0  oly:8
    SUMI r29, -76, r29 | AP -8(r29), r31                             ; @000000c0  oly:8
    SUMI r0, 1, r9                                                   ; @000000d0  oly:9
    CP -4(r30), r8                                                   ; @000000e0  oly:9
    NOP                                                              ; @000000f0
    NOP                                                              ; @00000100
    SMQ r9, r8, L_0                                                  ; @00000110  oly:9
    NOP                                                              ; @00000120
    NOP                                                              ; @00000130
    CP -4(r30), r8                                                   ; @00000140  oly:10
    NOP                                                              ; @00000150
    NOP                                                              ; @00000160
    SUM r8, r0, r1 | S f_fibonacci_fin                               ; @00000170  oly:10
L_0:
    CP -4(r30), r8                                                   ; @00000180  oly:12
    NOP                                                              ; @00000190
    NOP                                                              ; @000001a0
    RESTI r8, 1, r8                                                  ; @000001b0  oly:12
    NOP                                                              ; @000001c0
    NOP                                                              ; @000001d0
    AP -4(r29), r8 | SYE f_fibonacci                                 ; @000001e0  oly:12
    SUM r1, r0, r8                                                   ; @000001f0  oly:12
    NOP                                                              ; @00000200
    NOP                                                              ; @00000210
    AP -16(r30), r8                                                  ; @00000220  oly:12
    CP -4(r30), r9                                                   ; @00000230  oly:12
    NOP                                                              ; @00000240
    NOP                                                              ; @00000250
    RESTI r9, 2, r9                                                  ; @00000260  oly:12
    NOP                                                              ; @00000270
    NOP                                                              ; @00000280
    AP -4(r29), r9 | SYE f_fibonacci                                 ; @00000290  oly:12
    SUM r1, r0, r9 | CP -16(r30), r8                                 ; @000002a0  oly:12
    NOP                                                              ; @000002b0
    NOP                                                              ; @000002c0
    SUM r8, r9, r8                                                   ; @000002d0  oly:12
    NOP                                                              ; @000002e0
    NOP                                                              ; @000002f0
    SUM r8, r0, r1                                                   ; @00000300  oly:12
    NOP                                                              ; @00000310
    NOP                                                              ; @00000320
f_fibonacci_fin:
    SUM r30, r0, r29 | CP -8(r30), r31                               ; @00000330  oly:12
    CP -12(r30), r27                                                 ; @00000340  oly:12
    NOP                                                              ; @00000350
    NOP                                                              ; @00000360
    SUM r27, r0, r30 | SRG r31                                       ; @00000370  oly:12
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @00000380  oly:15
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @00000390  oly:15
    SUMI r0, 6, r8                                                   ; @000003a0  oly:16
    NOP                                                              ; @000003b0
    NOP                                                              ; @000003c0
    AP -4(r29), r8 | SYE f_fibonacci                                 ; @000003d0  oly:16
    SUM r1, r0, r8                                                   ; @000003e0  oly:16
    NOP                                                              ; @000003f0
    NOP                                                              ; @00000400
    AP 4(r28), r8                                                    ; @00000410  oly:16
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000420  oly:16
    CP -8(r30), r27                                                  ; @00000430  oly:16
    NOP                                                              ; @00000440
    NOP                                                              ; @00000450
    SUM r27, r0, r30 | SRG r31                                       ; @00000460  oly:16
