; generado por jaf desde ejercicios/suma_primos.oly
; un bundle por linea, slots 0..3 separados por '|'
; @xxxxxxxx es el byte del bundle en el .bin (cada bundle ocupa 16)
; oly:N es la linea del fuente que produjo la instruccion
_inicio:
    SUMI r0, 32, r28                                                 ; @00000000  oly:1
    SUMI r0, 512, r29                                                ; @00000010  oly:1
    SUMI r0, 0, r8                                                   ; @00000020  oly:6
    DLII r28, 11, r28                                                ; @00000030  oly:1
    DLII r29, 11, r29                                                ; @00000040  oly:1
    SUMI r0, 1, r10                                                  ; @00000050  oly:8
    SUMI r0, 0, r8 | AP 0(r28), r8                                   ; @00000060  oly:7,6
    SUM r29, r0, r30                                                 ; @00000070  oly:1
    SUMI r29, -64, r29                                               ; @00000080  oly:1
    SUMI r28, 8, r9 | AP 4(r28), r8                                  ; @00000090  oly:8,7
    SUMI r0, 10, r8                                                  ; @000000a0  oly:8
    NOP                                                              ; @000000b0
    NOP                                                              ; @000000c0
    SUMI r0, 1, r8 | AP 0(r9), r8                                    ; @000000d0  oly:8
    NOP                                                              ; @000000e0
    NOP                                                              ; @000000f0
    AP 4(r9), r8                                                     ; @00000100  oly:8
    SUMI r0, 2, r10 | AP 8(r9), r10                                  ; @00000110  oly:8
    NOP                                                              ; @00000120
    NOP                                                              ; @00000130
    SUMI r0, 3, r10 | AP 12(r9), r10                                 ; @00000140  oly:8
    NOP                                                              ; @00000150
    NOP                                                              ; @00000160
    SUMI r0, 4, r10 | AP 16(r9), r10                                 ; @00000170  oly:8
    NOP                                                              ; @00000180
    NOP                                                              ; @00000190
    SUMI r0, 5, r10 | AP 20(r9), r10                                 ; @000001a0  oly:8
    NOP                                                              ; @000001b0
    NOP                                                              ; @000001c0
    SUMI r0, 6, r10 | AP 24(r9), r10                                 ; @000001d0  oly:8
    NOP                                                              ; @000001e0
    NOP                                                              ; @000001f0
    SUMI r0, 7, r10 | AP 28(r9), r10                                 ; @00000200  oly:8
    NOP                                                              ; @00000210
    NOP                                                              ; @00000220
    SUMI r0, 8, r10 | AP 32(r9), r10                                 ; @00000230  oly:8
    NOP                                                              ; @00000240
    NOP                                                              ; @00000250
    SUMI r0, 9, r10 | AP 36(r9), r10                                 ; @00000260  oly:8
    NOP                                                              ; @00000270
    NOP                                                              ; @00000280
    SUMI r0, 11, r10 | AP 40(r9), r10                                ; @00000290  oly:8
    NOP                                                              ; @000002a0
    NOP                                                              ; @000002b0
    AP 44(r9), r10 | SYE f_main                                      ; @000002c0  oly:8
    FIN                                                              ; @000002d0  oly:8
f_es_primo:
    SUM r29, r0, r30 | AP -16(r29), r30                              ; @000002e0  oly:10
    SUMI r29, -80, r29 | AP -12(r29), r31                            ; @000002f0  oly:10
    SUMI r0, 1, r9                                                   ; @00000300  oly:11
    CP -4(r30), r8                                                   ; @00000310  oly:11
    NOP                                                              ; @00000320
    NOP                                                              ; @00000330
    SMQ r9, r8, L_0                                                  ; @00000340  oly:11
    NOP                                                              ; @00000350
    NOP                                                              ; @00000360
    SUMI r0, 0, r8                                                   ; @00000370  oly:12
    NOP                                                              ; @00000380
    NOP                                                              ; @00000390
    SUM r8, r0, r1 | S f_es_primo_fin                                ; @000003a0  oly:12
L_0:
    CP -8(r30), r8                                                   ; @000003b0  oly:14
    CP -8(r30), r9                                                   ; @000003c0  oly:14
    NOP                                                              ; @000003d0
    NOP                                                              ; @000003e0
    MUL r8, r9, r8 | CP -4(r30), r9                                  ; @000003f0  oly:14
    NOP                                                              ; @00000400
    NOP                                                              ; @00000410
    SMI r9, r8, L_1                                                  ; @00000420  oly:14
    NOP                                                              ; @00000430
    NOP                                                              ; @00000440
    SUMI r0, 1, r8                                                   ; @00000450  oly:15
    NOP                                                              ; @00000460
    NOP                                                              ; @00000470
    SUM r8, r0, r1 | S f_es_primo_fin                                ; @00000480  oly:15
L_1:
    CP -4(r30), r8                                                   ; @00000490  oly:17
    CP -8(r30), r9                                                   ; @000004a0  oly:17
    NOP                                                              ; @000004b0
    NOP                                                              ; @000004c0
    DIV r8, r9, r10                                                  ; @000004d0  oly:17
    NOP                                                              ; @000004e0
    NOP                                                              ; @000004f0
    MUL r10, r9, r10                                                 ; @00000500  oly:17
    NOP                                                              ; @00000510
    NOP                                                              ; @00000520
    REST r8, r10, r8                                                 ; @00000530  oly:17
    NOP                                                              ; @00000540
    NOP                                                              ; @00000550
    SNIG r8, r0, L_2                                                 ; @00000560  oly:17
    NOP                                                              ; @00000570
    NOP                                                              ; @00000580
    SUMI r0, 0, r8                                                   ; @00000590  oly:18
    NOP                                                              ; @000005a0
    NOP                                                              ; @000005b0
    SUM r8, r0, r1 | S f_es_primo_fin                                ; @000005c0  oly:18
L_2:
    CP -4(r30), r8                                                   ; @000005d0  oly:20
    CP -8(r30), r9                                                   ; @000005e0  oly:20
    NOP                                                              ; @000005f0
    AP -4(r29), r8                                                   ; @00000600  oly:20
    SUMI r9, 1, r9                                                   ; @00000610  oly:20
    NOP                                                              ; @00000620
    NOP                                                              ; @00000630
    AP -8(r29), r9 | SYE f_es_primo                                  ; @00000640  oly:20
    SUM r1, r0, r8                                                   ; @00000650  oly:20
    NOP                                                              ; @00000660
    NOP                                                              ; @00000670
    SUM r8, r0, r1                                                   ; @00000680  oly:20
    NOP                                                              ; @00000690
    NOP                                                              ; @000006a0
f_es_primo_fin:
    SUM r30, r0, r29 | CP -12(r30), r31                              ; @000006b0  oly:20
    CP -16(r30), r27                                                 ; @000006c0  oly:20
    NOP                                                              ; @000006d0
    NOP                                                              ; @000006e0
    SUM r27, r0, r30 | SRG r31                                       ; @000006f0  oly:20
f_sume:
    SUM r29, r0, r30 | AP -16(r29), r30                              ; @00000700  oly:23
    SUMI r29, -84, r29 | AP -12(r29), r31                            ; @00000710  oly:23
    SUMI r0, 0, r8                                                   ; @00000720  oly:24
    NOP                                                              ; @00000730
    NOP                                                              ; @00000740
    AP -20(r30), r8 | S L_3                                          ; @00000750  oly:24
L_4:
    CP -4(r30), r8                                                   ; @00000760  oly:25
    CP -20(r30), r9                                                  ; @00000770  oly:25
    NOP                                                              ; @00000780
    NOP                                                              ; @00000790
    DLII r9, 2, r9                                                   ; @000007a0  oly:25
    NOP                                                              ; @000007b0
    NOP                                                              ; @000007c0
    SUM r8, r9, r8                                                   ; @000007d0  oly:25
    SUMI r0, 2, r9                                                   ; @000007e0  oly:25
    NOP                                                              ; @000007f0
    CP 8(r8), r8                                                     ; @00000800  oly:25
    NOP                                                              ; @00000810
    NOP                                                              ; @00000820
    AP -4(r29), r8                                                   ; @00000830  oly:25
    AP -8(r29), r9 | SYE f_es_primo                                  ; @00000840  oly:25
    SUM r1, r0, r8                                                   ; @00000850  oly:25
    NOP                                                              ; @00000860
    NOP                                                              ; @00000870
    SIG r8, r0, L_5                                                  ; @00000880  oly:25
    NOP                                                              ; @00000890
    NOP                                                              ; @000008a0
    CP 0(r28), r8                                                    ; @000008b0  oly:26
    CP -4(r30), r9                                                   ; @000008c0  oly:26
    CP -20(r30), r10                                                 ; @000008d0  oly:26
    NOP                                                              ; @000008e0
    NOP                                                              ; @000008f0
    DLII r10, 2, r10                                                 ; @00000900  oly:26
    NOP                                                              ; @00000910
    NOP                                                              ; @00000920
    SUM r9, r10, r9                                                  ; @00000930  oly:26
    NOP                                                              ; @00000940
    NOP                                                              ; @00000950
    CP 8(r9), r9                                                     ; @00000960  oly:26
    NOP                                                              ; @00000970
    NOP                                                              ; @00000980
    SUM r8, r9, r8                                                   ; @00000990  oly:26
    NOP                                                              ; @000009a0
    NOP                                                              ; @000009b0
    AP 0(r28), r8                                                    ; @000009c0  oly:26
L_5:
    CP -20(r30), r8                                                  ; @000009d0  oly:24
    NOP                                                              ; @000009e0
    NOP                                                              ; @000009f0
    SUMI r8, 1, r8                                                   ; @00000a00  oly:24
    NOP                                                              ; @00000a10
    NOP                                                              ; @00000a20
    AP -20(r30), r8                                                  ; @00000a30  oly:24
L_3:
    CP -20(r30), r8                                                  ; @00000a40  oly:24
    CP -8(r30), r9                                                   ; @00000a50  oly:24
    NOP                                                              ; @00000a60
    NOP                                                              ; @00000a70
    SMQ r8, r9, L_4                                                  ; @00000a80  oly:24
    NOP                                                              ; @00000a90
    NOP                                                              ; @00000aa0
    CP 0(r28), r8                                                    ; @00000ab0  oly:29
    NOP                                                              ; @00000ac0
    NOP                                                              ; @00000ad0
    SUM r8, r0, r1                                                   ; @00000ae0  oly:29
    NOP                                                              ; @00000af0
    NOP                                                              ; @00000b00
f_sume_fin:
    SUM r30, r0, r29 | CP -12(r30), r31                              ; @00000b10  oly:29
    CP -16(r30), r27                                                 ; @00000b20  oly:29
    NOP                                                              ; @00000b30
    NOP                                                              ; @00000b40
    SUM r27, r0, r30 | SRG r31                                       ; @00000b50  oly:29
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @00000b60  oly:32
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @00000b70  oly:32
    SUMI r28, 8, r8                                                  ; @00000b80  oly:33
    SUMI r0, 10, r9                                                  ; @00000b90  oly:33
    NOP                                                              ; @00000ba0
    AP -4(r29), r8                                                   ; @00000bb0  oly:33
    AP -8(r29), r9 | SYE f_sume                                      ; @00000bc0  oly:33
    SUM r1, r0, r8                                                   ; @00000bd0  oly:33
    NOP                                                              ; @00000be0
    NOP                                                              ; @00000bf0
    AP 4(r28), r8                                                    ; @00000c00  oly:33
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000c10  oly:33
    CP -8(r30), r27                                                  ; @00000c20  oly:33
    NOP                                                              ; @00000c30
    NOP                                                              ; @00000c40
    SUM r27, r0, r30 | SRG r31                                       ; @00000c50  oly:33
