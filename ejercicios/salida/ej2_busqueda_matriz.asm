; generado por jaf desde ejercicios/ej2_busqueda_matriz.oly
; un bundle por linea, slots 0..3 separados por '|'
; @xxxxxxxx es el byte del bundle en el .bin (cada bundle ocupa 16)
; oly:N es la linea del fuente que produjo la instruccion
_inicio:
    SUMI r0, 32, r28                                                 ; @00000000  oly:1
    SUMI r0, 512, r29                                                ; @00000010  oly:1
    SUMI r0, 3, r8                                                   ; @00000020  oly:5
    DLII r28, 11, r28                                                ; @00000030  oly:1
    DLII r29, 11, r29                                                ; @00000040  oly:1
    SUMI r0, 3, r10                                                  ; @00000050  oly:5
    SUMI r28, 0, r9                                                  ; @00000060  oly:5
    SUM r29, r0, r30                                                 ; @00000070  oly:1
    SUMI r29, -64, r29                                               ; @00000080  oly:1
    SUMI r0, 4, r8 | AP 0(r9), r8                                    ; @00000090  oly:5
    NOP                                                              ; @000000a0
    NOP                                                              ; @000000b0
    SUMI r0, 2, r8 | AP 4(r9), r8                                    ; @000000c0  oly:6,5
    SUMI r0, 8, r10 | AP 8(r9), r10                                  ; @000000d0  oly:5
    NOP                                                              ; @000000e0
    NOP                                                              ; @000000f0
    SUMI r0, 1, r10 | AP 12(r9), r10                                 ; @00000100  oly:5
    NOP                                                              ; @00000110
    NOP                                                              ; @00000120
    SUMI r0, 7, r10 | AP 16(r9), r10                                 ; @00000130  oly:5
    NOP                                                              ; @00000140
    NOP                                                              ; @00000150
    SUMI r0, 9, r10 | AP 20(r9), r10                                 ; @00000160  oly:5
    NOP                                                              ; @00000170
    NOP                                                              ; @00000180
    SUMI r0, 4, r10 | AP 24(r9), r10                                 ; @00000190  oly:5
    NOP                                                              ; @000001a0
    NOP                                                              ; @000001b0
    SUMI r0, 6, r10 | AP 28(r9), r10                                 ; @000001c0  oly:5
    NOP                                                              ; @000001d0
    NOP                                                              ; @000001e0
    SUMI r0, 2, r10 | AP 32(r9), r10                                 ; @000001f0  oly:5
    NOP                                                              ; @00000200
    NOP                                                              ; @00000210
    SUMI r0, 5, r10 | AP 36(r9), r10                                 ; @00000220  oly:5
    NOP                                                              ; @00000230
    NOP                                                              ; @00000240
    SUMI r0, 0, r10 | AP 40(r9), r10                                 ; @00000250  oly:5
    NOP                                                              ; @00000260
    NOP                                                              ; @00000270
    SUMI r0, 12, r10 | AP 44(r9), r10                                ; @00000280  oly:5
    NOP                                                              ; @00000290
    NOP                                                              ; @000002a0
    SUMI r0, 11, r10 | AP 48(r9), r10                                ; @000002b0  oly:5
    NOP                                                              ; @000002c0
    NOP                                                              ; @000002d0
    SUMI r28, 56, r9 | AP 52(r9), r10                                ; @000002e0  oly:6,5
    NOP                                                              ; @000002f0
    NOP                                                              ; @00000300
    SUMI r0, 1, r8 | AP 0(r9), r8                                    ; @00000310  oly:6
    NOP                                                              ; @00000320
    NOP                                                              ; @00000330
    SUMI r0, 0, r8 | AP 4(r9), r8                                    ; @00000340  oly:7,6
    AP 8(r9), r0                                                     ; @00000350  oly:6
    AP 12(r9), r0                                                    ; @00000360  oly:6
    SUMI r0, 1, r8 | AP 72(r28), r8                                  ; @00000370  oly:8,7
    NOP                                                              ; @00000380
    NOP                                                              ; @00000390
    AP 76(r28), r8 | SYE f_main                                      ; @000003a0  oly:8
    FIN                                                              ; @000003b0  oly:8
f_buscar:
    SUM r29, r0, r30 | AP -28(r29), r30                              ; @000003c0  oly:10
    SUMI r29, -104, r29 | AP -24(r29), r31                           ; @000003d0  oly:10
    SUMI r0, 0, r8                                                   ; @000003e0  oly:11
    NOP                                                              ; @000003f0
    NOP                                                              ; @00000400
    SUMI r0, 0, r8 | AP -32(r30), r8                                 ; @00000410  oly:12,11
    NOP                                                              ; @00000420
    NOP                                                              ; @00000430
    AP -36(r30), r8 | S L_0                                          ; @00000440  oly:12,24
L_1:
    SUMI r0, 0, r8                                                   ; @00000450  oly:14
    NOP                                                              ; @00000460
    NOP                                                              ; @00000470
    AP -40(r30), r8 | S L_2                                          ; @00000480  oly:14,22
L_3:
    CP -4(r30), r8                                                   ; @00000490  oly:16
    CP -36(r30), r9                                                  ; @000004a0  oly:16
    CP -40(r30), r10                                                 ; @000004b0  oly:16
    CP 4(r8), r11                                                    ; @000004c0  oly:16
    NOP                                                              ; @000004d0
    NOP                                                              ; @000004e0
    MUL r9, r11, r9                                                  ; @000004f0  oly:16
    NOP                                                              ; @00000500
    NOP                                                              ; @00000510
    SUM r9, r10, r9                                                  ; @00000520  oly:16
    NOP                                                              ; @00000530
    NOP                                                              ; @00000540
    DLII r9, 2, r9                                                   ; @00000550  oly:16
    NOP                                                              ; @00000560
    NOP                                                              ; @00000570
    SUM r8, r9, r8                                                   ; @00000580  oly:16
    NOP                                                              ; @00000590
    NOP                                                              ; @000005a0
    CP 8(r8), r8                                                     ; @000005b0  oly:16
    CP -16(r30), r9                                                  ; @000005c0  oly:16
    NOP                                                              ; @000005d0
    NOP                                                              ; @000005e0
    SNIG r8, r9, L_4                                                 ; @000005f0  oly:16
    NOP                                                              ; @00000600
    NOP                                                              ; @00000610
    SUMI r0, 1, r8                                                   ; @00000620  oly:17
    SUMI r0, 0, r9                                                   ; @00000630  oly:18
    NOP                                                              ; @00000640
    AP -32(r30), r8                                                  ; @00000650  oly:17
    DLII r9, 2, r9 | CP -20(r30), r8                                 ; @00000660  oly:18
    NOP                                                              ; @00000670
    NOP                                                              ; @00000680
    SUM r8, r9, r8 | CP -36(r30), r9                                 ; @00000690  oly:18
    NOP                                                              ; @000006a0
    NOP                                                              ; @000006b0
    SUMI r0, 1, r9 | AP 8(r8), r9                                    ; @000006c0  oly:19,18
    CP -20(r30), r8                                                  ; @000006d0  oly:19
    NOP                                                              ; @000006e0
    DLII r9, 2, r9                                                   ; @000006f0  oly:19
    NOP                                                              ; @00000700
    NOP                                                              ; @00000710
    SUM r8, r9, r8 | CP -40(r30), r9                                 ; @00000720  oly:19
    NOP                                                              ; @00000730
    NOP                                                              ; @00000740
    AP 8(r8), r9                                                     ; @00000750  oly:19
L_4:
    CP -40(r30), r8                                                  ; @00000760  oly:21
    NOP                                                              ; @00000770
    NOP                                                              ; @00000780
    SUMI r8, 1, r8                                                   ; @00000790  oly:21
    NOP                                                              ; @000007a0
    NOP                                                              ; @000007b0
    AP -40(r30), r8                                                  ; @000007c0  oly:21
L_2:
    CP -40(r30), r8                                                  ; @000007d0  oly:15
    CP -12(r30), r9                                                  ; @000007e0  oly:15
    NOP                                                              ; @000007f0
    NOP                                                              ; @00000800
    SMI r8, r9, L_5                                                  ; @00000810  oly:15
    NOP                                                              ; @00000820
    NOP                                                              ; @00000830
    CP -32(r30), r8                                                  ; @00000840  oly:15
    NOP                                                              ; @00000850
    NOP                                                              ; @00000860
    SIG r8, r0, L_3                                                  ; @00000870  oly:15
    NOP                                                              ; @00000880
    NOP                                                              ; @00000890
L_5:
    CP -36(r30), r8                                                  ; @000008a0  oly:23
    NOP                                                              ; @000008b0
    NOP                                                              ; @000008c0
    SUMI r8, 1, r8                                                   ; @000008d0  oly:23
    NOP                                                              ; @000008e0
    NOP                                                              ; @000008f0
    AP -36(r30), r8                                                  ; @00000900  oly:23
L_0:
    CP -36(r30), r8                                                  ; @00000910  oly:13
    CP -8(r30), r9                                                   ; @00000920  oly:13
    NOP                                                              ; @00000930
    NOP                                                              ; @00000940
    SMI r8, r9, L_6                                                  ; @00000950  oly:13
    NOP                                                              ; @00000960
    NOP                                                              ; @00000970
    CP -32(r30), r8                                                  ; @00000980  oly:13
    NOP                                                              ; @00000990
    NOP                                                              ; @000009a0
    SIG r8, r0, L_1                                                  ; @000009b0  oly:13
    NOP                                                              ; @000009c0
    NOP                                                              ; @000009d0
L_6:
    CP -32(r30), r8                                                  ; @000009e0  oly:25
    NOP                                                              ; @000009f0
    NOP                                                              ; @00000a00
    SUM r8, r0, r1                                                   ; @00000a10  oly:25
    NOP                                                              ; @00000a20
    NOP                                                              ; @00000a30
f_buscar_fin:
    SUM r30, r0, r29 | CP -24(r30), r31                              ; @00000a40  oly:25
    CP -28(r30), r27                                                 ; @00000a50  oly:25
    NOP                                                              ; @00000a60
    NOP                                                              ; @00000a70
    SUM r27, r0, r30 | SRG r31                                       ; @00000a80  oly:25
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @00000a90  oly:28
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @00000aa0  oly:28
    SUMI r28, 0, r8                                                  ; @00000ab0  oly:29
    SUMI r0, 3, r9                                                   ; @00000ac0  oly:29
    SUMI r0, 4, r10                                                  ; @00000ad0  oly:29
    SUMI r0, 6, r11 | AP -4(r29), r8                                 ; @00000ae0  oly:29
    SUMI r28, 56, r12 | AP -8(r29), r9                               ; @00000af0  oly:29
    AP -12(r29), r10                                                 ; @00000b00  oly:29
    AP -16(r29), r11                                                 ; @00000b10  oly:29
    AP -20(r29), r12 | SYE f_buscar                                  ; @00000b20  oly:29
    SUM r1, r0, r8                                                   ; @00000b30  oly:29
    SUMI r0, 3, r9                                                   ; @00000b40  oly:30
    SUMI r0, 4, r10                                                  ; @00000b50  oly:30
    SUMI r28, 0, r8 | AP 72(r28), r8                                 ; @00000b60  oly:30,29
    SUMI r0, 99, r11                                                 ; @00000b70  oly:30
    SUMI r28, 56, r12                                                ; @00000b80  oly:30
    AP -4(r29), r8                                                   ; @00000b90  oly:30
    AP -8(r29), r9                                                   ; @00000ba0  oly:30
    AP -12(r29), r10                                                 ; @00000bb0  oly:30
    AP -16(r29), r11                                                 ; @00000bc0  oly:30
    AP -20(r29), r12 | SYE f_buscar                                  ; @00000bd0  oly:30
    SUM r1, r0, r8                                                   ; @00000be0  oly:30
    NOP                                                              ; @00000bf0
    NOP                                                              ; @00000c00
    AP 76(r28), r8                                                   ; @00000c10  oly:30
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000c20  oly:30
    CP -8(r30), r27                                                  ; @00000c30  oly:30
    NOP                                                              ; @00000c40
    NOP                                                              ; @00000c50
    SUM r27, r0, r30 | SRG r31                                       ; @00000c60  oly:30
