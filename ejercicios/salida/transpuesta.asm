; generado por jaf desde ejercicios/transpuesta.oly
; un bundle por linea, slots 0..3 separados por '|'
; @xxxxxxxx es el byte del bundle en el .bin (cada bundle ocupa 16)
; oly:N es la linea del fuente que produjo la instruccion
_inicio:
    SUMI r0, 32, r28                                                 ; @00000000  oly:1
    SUMI r0, 512, r29                                                ; @00000010  oly:1
    SUMI r0, 3, r8                                                   ; @00000020  oly:6
    DLII r28, 11, r28                                                ; @00000030  oly:1
    DLII r29, 11, r29                                                ; @00000040  oly:1
    SUMI r0, 1, r10                                                  ; @00000050  oly:7
    SUMI r28, 0, r9                                                  ; @00000060  oly:6
    SUM r29, r0, r30                                                 ; @00000070  oly:1
    SUMI r29, -64, r29                                               ; @00000080  oly:1
    SUMI r0, 2, r8 | AP 0(r9), r8                                    ; @00000090  oly:6
    NOP                                                              ; @000000a0
    NOP                                                              ; @000000b0
    SUMI r0, 2, r8 | AP 4(r9), r8                                    ; @000000c0  oly:7,6
    AP 8(r9), r0                                                     ; @000000d0  oly:6
    AP 12(r9), r0                                                    ; @000000e0  oly:6
    AP 16(r9), r0                                                    ; @000000f0  oly:6
    AP 20(r9), r0                                                    ; @00000100  oly:6
    AP 24(r9), r0                                                    ; @00000110  oly:6
    SUMI r28, 32, r9 | AP 28(r9), r0                                 ; @00000120  oly:7,6
    NOP                                                              ; @00000130
    NOP                                                              ; @00000140
    SUMI r0, 3, r8 | AP 0(r9), r8                                    ; @00000150  oly:7
    NOP                                                              ; @00000160
    NOP                                                              ; @00000170
    AP 4(r9), r8                                                     ; @00000180  oly:7
    SUMI r0, 2, r10 | AP 8(r9), r10                                  ; @00000190  oly:7
    NOP                                                              ; @000001a0
    NOP                                                              ; @000001b0
    SUMI r0, 3, r10 | AP 12(r9), r10                                 ; @000001c0  oly:7
    NOP                                                              ; @000001d0
    NOP                                                              ; @000001e0
    SUMI r0, 4, r10 | AP 16(r9), r10                                 ; @000001f0  oly:7
    NOP                                                              ; @00000200
    NOP                                                              ; @00000210
    SUMI r0, 5, r10 | AP 20(r9), r10                                 ; @00000220  oly:7
    NOP                                                              ; @00000230
    NOP                                                              ; @00000240
    SUMI r0, 6, r10 | AP 24(r9), r10                                 ; @00000250  oly:7
    NOP                                                              ; @00000260
    NOP                                                              ; @00000270
    AP 28(r9), r10 | SYE f_main                                      ; @00000280  oly:7
    FIN                                                              ; @00000290  oly:7
f_transponer_matriz:
    SUM r29, r0, r30 | AP -24(r29), r30                              ; @000002a0  oly:9
    SUMI r29, -96, r29 | AP -20(r29), r31                            ; @000002b0  oly:9
    SUMI r0, 0, r8                                                   ; @000002c0  oly:10
    NOP                                                              ; @000002d0
    NOP                                                              ; @000002e0
    AP -28(r30), r8 | S L_0                                          ; @000002f0  oly:10
L_1:
    SUMI r0, 0, r8                                                   ; @00000300  oly:11
    NOP                                                              ; @00000310
    NOP                                                              ; @00000320
    AP -32(r30), r8 | S L_2                                          ; @00000330  oly:11
L_3:
    CP -16(r30), r8                                                  ; @00000340  oly:12
    CP -28(r30), r9                                                  ; @00000350  oly:12
    CP -32(r30), r10                                                 ; @00000360  oly:12
    CP 4(r8), r11                                                    ; @00000370  oly:12
    NOP                                                              ; @00000380
    NOP                                                              ; @00000390
    MUL r9, r11, r9                                                  ; @000003a0  oly:12
    NOP                                                              ; @000003b0
    NOP                                                              ; @000003c0
    SUM r9, r10, r9                                                  ; @000003d0  oly:12
    NOP                                                              ; @000003e0
    NOP                                                              ; @000003f0
    DLII r9, 2, r9                                                   ; @00000400  oly:12
    NOP                                                              ; @00000410
    NOP                                                              ; @00000420
    SUM r8, r9, r8 | CP -4(r30), r9                                  ; @00000430  oly:12
    CP -32(r30), r10                                                 ; @00000440  oly:12
    CP -28(r30), r11                                                 ; @00000450  oly:12
    CP 4(r9), r12                                                    ; @00000460  oly:12
    NOP                                                              ; @00000470
    NOP                                                              ; @00000480
    MUL r10, r12, r10                                                ; @00000490  oly:12
    NOP                                                              ; @000004a0
    NOP                                                              ; @000004b0
    SUM r10, r11, r10                                                ; @000004c0  oly:12
    NOP                                                              ; @000004d0
    NOP                                                              ; @000004e0
    DLII r10, 2, r10                                                 ; @000004f0  oly:12
    NOP                                                              ; @00000500
    NOP                                                              ; @00000510
    SUM r9, r10, r9                                                  ; @00000520  oly:12
    NOP                                                              ; @00000530
    NOP                                                              ; @00000540
    CP 8(r9), r9                                                     ; @00000550  oly:12
    NOP                                                              ; @00000560
    NOP                                                              ; @00000570
    AP 8(r8), r9                                                     ; @00000580  oly:12
    CP -32(r30), r8                                                  ; @00000590  oly:11
    NOP                                                              ; @000005a0
    NOP                                                              ; @000005b0
    SUMI r8, 1, r8                                                   ; @000005c0  oly:11
    NOP                                                              ; @000005d0
    NOP                                                              ; @000005e0
    AP -32(r30), r8                                                  ; @000005f0  oly:11
L_2:
    CP -32(r30), r8                                                  ; @00000600  oly:11
    CP -8(r30), r9                                                   ; @00000610  oly:11
    NOP                                                              ; @00000620
    NOP                                                              ; @00000630
    SMQ r8, r9, L_3                                                  ; @00000640  oly:11
    NOP                                                              ; @00000650
    NOP                                                              ; @00000660
    CP -28(r30), r8                                                  ; @00000670  oly:10
    NOP                                                              ; @00000680
    NOP                                                              ; @00000690
    SUMI r8, 1, r8                                                   ; @000006a0  oly:10
    NOP                                                              ; @000006b0
    NOP                                                              ; @000006c0
    AP -28(r30), r8                                                  ; @000006d0  oly:10
L_0:
    CP -28(r30), r8                                                  ; @000006e0  oly:10
    CP -12(r30), r9                                                  ; @000006f0  oly:10
    NOP                                                              ; @00000700
    NOP                                                              ; @00000710
    SMQ r8, r9, L_1                                                  ; @00000720  oly:10
    NOP                                                              ; @00000730
    NOP                                                              ; @00000740
f_transponer_matriz_fin:
    SUM r30, r0, r29 | CP -20(r30), r31                              ; @00000750  oly:10
    CP -24(r30), r27                                                 ; @00000760  oly:10
    NOP                                                              ; @00000770
    NOP                                                              ; @00000780
    SUM r27, r0, r30 | SRG r31                                       ; @00000790  oly:10
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @000007a0  oly:17
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @000007b0  oly:17
    SUMI r28, 32, r8                                                 ; @000007c0  oly:18
    SUMI r0, 2, r9                                                   ; @000007d0  oly:18
    SUMI r0, 3, r10                                                  ; @000007e0  oly:18
    SUMI r28, 0, r11 | AP -4(r29), r8                                ; @000007f0  oly:18
    AP -8(r29), r9                                                   ; @00000800  oly:18
    AP -12(r29), r10                                                 ; @00000810  oly:18
    AP -16(r29), r11 | SYE f_transponer_matriz                       ; @00000820  oly:18
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000830  oly:18
    CP -8(r30), r27                                                  ; @00000840  oly:18
    NOP                                                              ; @00000850
    NOP                                                              ; @00000860
    SUM r27, r0, r30 | SRG r31                                       ; @00000870  oly:18
