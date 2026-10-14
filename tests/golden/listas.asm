; generado por jaf desde tests/golden/listas.oly
; un bundle por linea, slots 0..3 separados por '|'
; @xxxxxxxx es el byte del bundle en el .bin (cada bundle ocupa 16)
; oly:N es la linea del fuente que produjo la instruccion
_inicio:
    SUMI r0, 32, r28                                                 ; @00000000  oly:1
    SUMI r0, 512, r29                                                ; @00000010  oly:1
    SUMI r0, 10, r8                                                  ; @00000020  oly:1
    DLII r28, 11, r28                                                ; @00000030  oly:1
    DLII r29, 11, r29                                                ; @00000040  oly:1
    SUMI r0, 1, r10                                                  ; @00000050  oly:3
    SUMI r28, 0, r9                                                  ; @00000060  oly:1
    SUM r29, r0, r30                                                 ; @00000070  oly:1
    SUMI r29, -64, r29                                               ; @00000080  oly:1
    SUMI r0, 1, r8 | AP 0(r9), r8                                    ; @00000090  oly:1
    NOP                                                              ; @000000a0
    NOP                                                              ; @000000b0
    SUMI r0, 3, r8 | AP 4(r9), r8                                    ; @000000c0  oly:2,1
    AP 8(r9), r0                                                     ; @000000d0  oly:1
    AP 12(r9), r0                                                    ; @000000e0  oly:1
    AP 16(r9), r0                                                    ; @000000f0  oly:1
    AP 20(r9), r0                                                    ; @00000100  oly:1
    AP 24(r9), r0                                                    ; @00000110  oly:1
    AP 28(r9), r0                                                    ; @00000120  oly:1
    AP 32(r9), r0                                                    ; @00000130  oly:1
    AP 36(r9), r0                                                    ; @00000140  oly:1
    AP 40(r9), r0                                                    ; @00000150  oly:1
    SUMI r28, 48, r9 | AP 44(r9), r0                                 ; @00000160  oly:2,1
    NOP                                                              ; @00000170
    NOP                                                              ; @00000180
    SUMI r0, 4, r8 | AP 0(r9), r8                                    ; @00000190  oly:2
    NOP                                                              ; @000001a0
    NOP                                                              ; @000001b0
    SUMI r0, 3, r8 | AP 4(r9), r8                                    ; @000001c0  oly:3,2
    AP 8(r9), r0                                                     ; @000001d0  oly:2
    AP 12(r9), r0                                                    ; @000001e0  oly:2
    AP 16(r9), r0                                                    ; @000001f0  oly:2
    AP 20(r9), r0                                                    ; @00000200  oly:2
    AP 24(r9), r0                                                    ; @00000210  oly:2
    AP 28(r9), r0                                                    ; @00000220  oly:2
    AP 32(r9), r0                                                    ; @00000230  oly:2
    AP 36(r9), r0                                                    ; @00000240  oly:2
    AP 40(r9), r0                                                    ; @00000250  oly:2
    AP 44(r9), r0                                                    ; @00000260  oly:2
    AP 48(r9), r0                                                    ; @00000270  oly:2
    SUMI r28, 104, r9 | AP 52(r9), r0                                ; @00000280  oly:3,2
    NOP                                                              ; @00000290
    NOP                                                              ; @000002a0
    SUMI r0, 1, r8 | AP 0(r9), r8                                    ; @000002b0  oly:3
    NOP                                                              ; @000002c0
    NOP                                                              ; @000002d0
    SUMI r0, 2, r8 | AP 4(r9), r8                                    ; @000002e0  oly:4,3
    SUMI r0, 42, r10 | AP 8(r9), r10                                 ; @000002f0  oly:3
    NOP                                                              ; @00000300
    NOP                                                              ; @00000310
    SUMI r0, 3, r10 | AP 12(r9), r10                                 ; @00000320  oly:3
    NOP                                                              ; @00000330
    NOP                                                              ; @00000340
    SUMI r28, 124, r9 | AP 16(r9), r10                               ; @00000350  oly:4,3
    SUMI r0, 1, r10                                                  ; @00000360  oly:4
    NOP                                                              ; @00000370
    SUMI r0, 2, r8 | AP 0(r9), r8                                    ; @00000380  oly:4
    NOP                                                              ; @00000390
    NOP                                                              ; @000003a0
    AP 4(r9), r8                                                     ; @000003b0  oly:4
    SUMI r0, 2, r10 | AP 8(r9), r10                                  ; @000003c0  oly:4
    NOP                                                              ; @000003d0
    NOP                                                              ; @000003e0
    SUMI r0, 3, r10 | AP 12(r9), r10                                 ; @000003f0  oly:4
    NOP                                                              ; @00000400
    NOP                                                              ; @00000410
    SUMI r0, 4, r10 | AP 16(r9), r10                                 ; @00000420  oly:4
    NOP                                                              ; @00000430
    NOP                                                              ; @00000440
    AP 20(r9), r10 | SYE f_main                                      ; @00000450  oly:4
    FIN                                                              ; @00000460  oly:4
f_f:
    SUM r29, r0, r30 | AP -16(r29), r30                              ; @00000470  oly:5
    SUMI r29, -88, r29 | AP -12(r29), r31                            ; @00000480  oly:5
    SUMI r0, 0, r8                                                   ; @00000490  oly:6
    NOP                                                              ; @000004a0
    NOP                                                              ; @000004b0
    SUMI r0, 0, r8 | AP -20(r30), r8                                 ; @000004c0  oly:7,6
    NOP                                                              ; @000004d0
    NOP                                                              ; @000004e0
    AP -24(r30), r8 | S L_0                                          ; @000004f0  oly:7
L_1:
    CP -20(r30), r8                                                  ; @00000500  oly:8
    CP -4(r30), r9                                                   ; @00000510  oly:8
    CP -24(r30), r10                                                 ; @00000520  oly:8
    NOP                                                              ; @00000530
    NOP                                                              ; @00000540
    DLII r10, 2, r10                                                 ; @00000550  oly:8
    NOP                                                              ; @00000560
    NOP                                                              ; @00000570
    SUM r9, r10, r9                                                  ; @00000580  oly:8
    NOP                                                              ; @00000590
    NOP                                                              ; @000005a0
    CP 8(r9), r9                                                     ; @000005b0  oly:8
    NOP                                                              ; @000005c0
    NOP                                                              ; @000005d0
    SUM r8, r9, r8                                                   ; @000005e0  oly:8
    NOP                                                              ; @000005f0
    NOP                                                              ; @00000600
    AP -20(r30), r8                                                  ; @00000610  oly:8
    CP -24(r30), r8                                                  ; @00000620  oly:7
    NOP                                                              ; @00000630
    NOP                                                              ; @00000640
    SUMI r8, 1, r8                                                   ; @00000650  oly:7
    NOP                                                              ; @00000660
    NOP                                                              ; @00000670
    AP -24(r30), r8                                                  ; @00000680  oly:7
L_0:
    CP -24(r30), r8                                                  ; @00000690  oly:7
    CP -8(r30), r9                                                   ; @000006a0  oly:7
    NOP                                                              ; @000006b0
    NOP                                                              ; @000006c0
    SMQ r8, r9, L_1                                                  ; @000006d0  oly:7
    NOP                                                              ; @000006e0
    NOP                                                              ; @000006f0
    CP -20(r30), r8                                                  ; @00000700  oly:10
    NOP                                                              ; @00000710
    NOP                                                              ; @00000720
    SUM r8, r0, r1                                                   ; @00000730  oly:10
    NOP                                                              ; @00000740
    NOP                                                              ; @00000750
f_f_fin:
    SUM r30, r0, r29 | CP -12(r30), r31                              ; @00000760  oly:10
    CP -16(r30), r27                                                 ; @00000770  oly:10
    NOP                                                              ; @00000780
    NOP                                                              ; @00000790
    SUM r27, r0, r30 | SRG r31                                       ; @000007a0  oly:10
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @000007b0  oly:12
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @000007c0  oly:12
    SUMI r28, 0, r8                                                  ; @000007d0  oly:13
    SUMI r0, 0, r9                                                   ; @000007e0  oly:13
    SUMI r0, 2, r10                                                  ; @000007f0  oly:14
    NOP                                                              ; @00000800
    DLII r9, 2, r9                                                   ; @00000810  oly:13
    NOP                                                              ; @00000820
    NOP                                                              ; @00000830
    SUM r8, r9, r8                                                   ; @00000840  oly:13
    SUMI r0, 5, r9                                                   ; @00000850  oly:13
    NOP                                                              ; @00000860
    NOP                                                              ; @00000870
    SUMI r28, 48, r8 | AP 8(r8), r9                                  ; @00000880  oly:14,13
    SUMI r0, 1, r9                                                   ; @00000890  oly:14
    NOP                                                              ; @000008a0
    CP 4(r8), r11                                                    ; @000008b0  oly:14
    NOP                                                              ; @000008c0
    NOP                                                              ; @000008d0
    MUL r9, r11, r9                                                  ; @000008e0  oly:14
    NOP                                                              ; @000008f0
    NOP                                                              ; @00000900
    SUM r9, r10, r9                                                  ; @00000910  oly:14
    SUMI r0, 0, r10                                                  ; @00000920  oly:14
    NOP                                                              ; @00000930
    DLII r9, 2, r9                                                   ; @00000940  oly:14
    DLII r10, 2, r10                                                 ; @00000950  oly:14
    NOP                                                              ; @00000960
    SUM r8, r9, r8                                                   ; @00000970  oly:14
    SUMI r28, 0, r9                                                  ; @00000980  oly:14
    NOP                                                              ; @00000990
    NOP                                                              ; @000009a0
    SUM r9, r10, r9                                                  ; @000009b0  oly:14
    SUMI r0, 2, r10                                                  ; @000009c0  oly:15
    NOP                                                              ; @000009d0
    CP 8(r9), r9                                                     ; @000009e0  oly:14
    NOP                                                              ; @000009f0
    NOP                                                              ; @00000a00
    SUMI r9, 255, r9                                                 ; @00000a10  oly:14
    NOP                                                              ; @00000a20
    NOP                                                              ; @00000a30
    SUMI r28, 48, r8 | AP 8(r8), r9                                  ; @00000a40  oly:15,14
    SUMI r0, 1, r9                                                   ; @00000a50  oly:15
    NOP                                                              ; @00000a60
    CP 4(r8), r11                                                    ; @00000a70  oly:15
    NOP                                                              ; @00000a80
    NOP                                                              ; @00000a90
    MUL r9, r11, r9                                                  ; @00000aa0  oly:15
    NOP                                                              ; @00000ab0
    NOP                                                              ; @00000ac0
    SUM r9, r10, r9                                                  ; @00000ad0  oly:15
    SUMI r0, 1, r10                                                  ; @00000ae0  oly:15
    NOP                                                              ; @00000af0
    DLII r9, 2, r9                                                   ; @00000b00  oly:15
    NOP                                                              ; @00000b10
    NOP                                                              ; @00000b20
    SUM r8, r9, r8                                                   ; @00000b30  oly:15
    NOP                                                              ; @00000b40
    NOP                                                              ; @00000b50
    CP 8(r8), r9                                                     ; @00000b60  oly:15
    NOP                                                              ; @00000b70
    NOP                                                              ; @00000b80
    REST r9, r10, r9                                                 ; @00000b90  oly:15
    NOP                                                              ; @00000ba0
    NOP                                                              ; @00000bb0
    AP 8(r8), r9 | S L_2                                             ; @00000bc0  oly:15,16
L_3:
    FIN                                                              ; @00000bd0  oly:16
L_2:
    S L_3                                                            ; @00000be0  oly:16
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000bf0  oly:16
    CP -8(r30), r27                                                  ; @00000c00  oly:16
    NOP                                                              ; @00000c10
    NOP                                                              ; @00000c20
    SUM r27, r0, r30 | SRG r31                                       ; @00000c30  oly:16
