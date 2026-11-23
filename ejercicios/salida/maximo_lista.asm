; generado por jaf desde ejercicios/maximo_lista.oly
; un bundle por linea, slots 0..3 separados por '|'
; @xxxxxxxx es el byte del bundle en el .bin (cada bundle ocupa 16)
; oly:N es la linea del fuente que produjo la instruccion
_inicio:
    SUMI r0, 32, r28                                                 ; @00000000  oly:1
    SUMI r0, 512, r29                                                ; @00000010  oly:1
    SUMI r0, 0, r8                                                   ; @00000020  oly:4
    DLII r28, 11, r28                                                ; @00000030  oly:1
    DLII r29, 11, r29                                                ; @00000040  oly:1
    SUMI r0, 3, r10                                                  ; @00000050  oly:5
    SUMI r28, 4, r9 | AP 0(r28), r8                                  ; @00000060  oly:5,4
    SUM r29, r0, r30                                                 ; @00000070  oly:1
    SUMI r29, -64, r29                                               ; @00000080  oly:1
    SUMI r0, 7, r8                                                   ; @00000090  oly:5
    NOP                                                              ; @000000a0
    NOP                                                              ; @000000b0
    SUMI r0, 1, r8 | AP 0(r9), r8                                    ; @000000c0  oly:5
    NOP                                                              ; @000000d0
    NOP                                                              ; @000000e0
    AP 4(r9), r8                                                     ; @000000f0  oly:5
    SUMI r0, 4, r10 | AP 8(r9), r10                                  ; @00000100  oly:5
    NOP                                                              ; @00000110
    NOP                                                              ; @00000120
    SUMI r0, 5, r10 | AP 12(r9), r10                                 ; @00000130  oly:5
    NOP                                                              ; @00000140
    NOP                                                              ; @00000150
    SUMI r0, 24, r10 | AP 16(r9), r10                                ; @00000160  oly:5
    NOP                                                              ; @00000170
    NOP                                                              ; @00000180
    SUMI r0, 5, r10 | AP 20(r9), r10                                 ; @00000190  oly:5
    NOP                                                              ; @000001a0
    NOP                                                              ; @000001b0
    SUMI r0, 65, r10 | AP 24(r9), r10                                ; @000001c0  oly:5
    NOP                                                              ; @000001d0
    NOP                                                              ; @000001e0
    SUMI r0, 46, r10 | AP 28(r9), r10                                ; @000001f0  oly:5
    NOP                                                              ; @00000200
    NOP                                                              ; @00000210
    AP 32(r9), r10 | SYE f_main                                      ; @00000220  oly:5
    FIN                                                              ; @00000230  oly:5
f_maximo_lista:
    SUM r29, r0, r30 | AP -16(r29), r30                              ; @00000240  oly:7
    SUMI r29, -88, r29 | AP -12(r29), r31                            ; @00000250  oly:7
    SUMI r0, 0, r9                                                   ; @00000260  oly:8
    CP -4(r30), r8                                                   ; @00000270  oly:8
    NOP                                                              ; @00000280
    DLII r9, 2, r9                                                   ; @00000290  oly:8
    NOP                                                              ; @000002a0
    NOP                                                              ; @000002b0
    SUM r8, r9, r8                                                   ; @000002c0  oly:8
    NOP                                                              ; @000002d0
    NOP                                                              ; @000002e0
    CP 8(r8), r8                                                     ; @000002f0  oly:8
    NOP                                                              ; @00000300
    NOP                                                              ; @00000310
    SUMI r0, 0, r8 | AP -20(r30), r8                                 ; @00000320  oly:9,8
    NOP                                                              ; @00000330
    NOP                                                              ; @00000340
    AP -24(r30), r8 | S L_0                                          ; @00000350  oly:9
L_1:
    CP -4(r30), r8                                                   ; @00000360  oly:10
    CP -24(r30), r9                                                  ; @00000370  oly:10
    NOP                                                              ; @00000380
    NOP                                                              ; @00000390
    DLII r9, 2, r9                                                   ; @000003a0  oly:10
    NOP                                                              ; @000003b0
    NOP                                                              ; @000003c0
    SUM r8, r9, r8                                                   ; @000003d0  oly:10
    NOP                                                              ; @000003e0
    NOP                                                              ; @000003f0
    CP 8(r8), r8                                                     ; @00000400  oly:10
    CP -20(r30), r9                                                  ; @00000410  oly:10
    NOP                                                              ; @00000420
    NOP                                                              ; @00000430
    SMI r9, r8, L_2                                                  ; @00000440  oly:10
    NOP                                                              ; @00000450
    NOP                                                              ; @00000460
    CP -4(r30), r8                                                   ; @00000470  oly:11
    CP -24(r30), r9                                                  ; @00000480  oly:11
    NOP                                                              ; @00000490
    NOP                                                              ; @000004a0
    DLII r9, 2, r9                                                   ; @000004b0  oly:11
    NOP                                                              ; @000004c0
    NOP                                                              ; @000004d0
    SUM r8, r9, r8                                                   ; @000004e0  oly:11
    NOP                                                              ; @000004f0
    NOP                                                              ; @00000500
    CP 8(r8), r8                                                     ; @00000510  oly:11
    NOP                                                              ; @00000520
    NOP                                                              ; @00000530
    AP -20(r30), r8                                                  ; @00000540  oly:11
L_2:
    CP -24(r30), r8                                                  ; @00000550  oly:9
    NOP                                                              ; @00000560
    NOP                                                              ; @00000570
    SUMI r8, 1, r8                                                   ; @00000580  oly:9
    NOP                                                              ; @00000590
    NOP                                                              ; @000005a0
    AP -24(r30), r8                                                  ; @000005b0  oly:9
L_0:
    CP -24(r30), r8                                                  ; @000005c0  oly:9
    CP -8(r30), r9                                                   ; @000005d0  oly:9
    NOP                                                              ; @000005e0
    NOP                                                              ; @000005f0
    SMQ r8, r9, L_1                                                  ; @00000600  oly:9
    NOP                                                              ; @00000610
    NOP                                                              ; @00000620
    CP -20(r30), r8                                                  ; @00000630  oly:14
    NOP                                                              ; @00000640
    NOP                                                              ; @00000650
    SUM r8, r0, r1                                                   ; @00000660  oly:14
    NOP                                                              ; @00000670
    NOP                                                              ; @00000680
f_maximo_lista_fin:
    SUM r30, r0, r29 | CP -12(r30), r31                              ; @00000690  oly:14
    CP -16(r30), r27                                                 ; @000006a0  oly:14
    NOP                                                              ; @000006b0
    NOP                                                              ; @000006c0
    SUM r27, r0, r30 | SRG r31                                       ; @000006d0  oly:14
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @000006e0  oly:17
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @000006f0  oly:17
    SUMI r28, 4, r8                                                  ; @00000700  oly:18
    SUMI r0, 7, r9                                                   ; @00000710  oly:18
    NOP                                                              ; @00000720
    AP -4(r29), r8                                                   ; @00000730  oly:18
    AP -8(r29), r9 | SYE f_maximo_lista                              ; @00000740  oly:18
    SUM r1, r0, r8                                                   ; @00000750  oly:18
    NOP                                                              ; @00000760
    NOP                                                              ; @00000770
    AP 0(r28), r8                                                    ; @00000780  oly:18
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000790  oly:18
    CP -8(r30), r27                                                  ; @000007a0  oly:18
    NOP                                                              ; @000007b0
    NOP                                                              ; @000007c0
    SUM r27, r0, r30 | SRG r31                                       ; @000007d0  oly:18
