; generado por jaf desde ejercicios/multiplicar_matrices.oly
; un bundle por linea, slots 0..3 separados por '|'
; @xxxxxxxx es el byte del bundle en el .bin (cada bundle ocupa 16)
; oly:N es la linea del fuente que produjo la instruccion
_inicio:
    SUMI r0, 32, r28                                                 ; @00000000  oly:1
    SUMI r0, 512, r29                                                ; @00000010  oly:1
    SUMI r0, 2, r8                                                   ; @00000020  oly:6
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
    SUMI r28, 24, r9 | AP 20(r9), r0                                 ; @00000100  oly:7,6
    NOP                                                              ; @00000110
    NOP                                                              ; @00000120
    SUMI r0, 2, r8 | AP 0(r9), r8                                    ; @00000130  oly:7
    NOP                                                              ; @00000140
    NOP                                                              ; @00000150
    SUMI r0, 2, r8 | AP 4(r9), r8                                    ; @00000160  oly:8,7
    SUMI r0, 2, r10 | AP 8(r9), r10                                  ; @00000170  oly:7
    NOP                                                              ; @00000180
    NOP                                                              ; @00000190
    SUMI r0, 3, r10 | AP 12(r9), r10                                 ; @000001a0  oly:7
    NOP                                                              ; @000001b0
    NOP                                                              ; @000001c0
    SUMI r0, 4, r10 | AP 16(r9), r10                                 ; @000001d0  oly:7
    NOP                                                              ; @000001e0
    NOP                                                              ; @000001f0
    SUMI r28, 48, r9 | AP 20(r9), r10                                ; @00000200  oly:8,7
    SUMI r0, 5, r10                                                  ; @00000210  oly:8
    NOP                                                              ; @00000220
    SUMI r0, 2, r8 | AP 0(r9), r8                                    ; @00000230  oly:8
    NOP                                                              ; @00000240
    NOP                                                              ; @00000250
    AP 4(r9), r8                                                     ; @00000260  oly:8
    SUMI r0, 6, r10 | AP 8(r9), r10                                  ; @00000270  oly:8
    NOP                                                              ; @00000280
    NOP                                                              ; @00000290
    SUMI r0, 7, r10 | AP 12(r9), r10                                 ; @000002a0  oly:8
    NOP                                                              ; @000002b0
    NOP                                                              ; @000002c0
    SUMI r0, 8, r10 | AP 16(r9), r10                                 ; @000002d0  oly:8
    NOP                                                              ; @000002e0
    NOP                                                              ; @000002f0
    AP 20(r9), r10 | SYE f_main                                      ; @00000300  oly:8
    FIN                                                              ; @00000310  oly:8
f_multiplicar_matrices:
    SUM r29, r0, r30 | AP -32(r29), r30                              ; @00000320  oly:10
    SUMI r29, -112, r29 | AP -28(r29), r31                           ; @00000330  oly:10
    SUMI r0, 0, r8                                                   ; @00000340  oly:11
    NOP                                                              ; @00000350
    NOP                                                              ; @00000360
    AP -36(r30), r8 | S L_0                                          ; @00000370  oly:11
L_1:
    SUMI r0, 0, r8                                                   ; @00000380  oly:12
    NOP                                                              ; @00000390
    NOP                                                              ; @000003a0
    AP -40(r30), r8 | S L_2                                          ; @000003b0  oly:12
L_3:
    SUMI r0, 0, r8                                                   ; @000003c0  oly:13
    NOP                                                              ; @000003d0
    NOP                                                              ; @000003e0
    SUMI r0, 0, r8 | AP -44(r30), r8                                 ; @000003f0  oly:14,13
    NOP                                                              ; @00000400
    NOP                                                              ; @00000410
    AP -48(r30), r8 | S L_4                                          ; @00000420  oly:14
L_5:
    CP -44(r30), r8                                                  ; @00000430  oly:15
    CP -4(r30), r9                                                   ; @00000440  oly:15
    CP -36(r30), r10                                                 ; @00000450  oly:15
    CP -48(r30), r11                                                 ; @00000460  oly:15
    CP 4(r9), r12                                                    ; @00000470  oly:15
    NOP                                                              ; @00000480
    NOP                                                              ; @00000490
    MUL r10, r12, r10                                                ; @000004a0  oly:15
    NOP                                                              ; @000004b0
    NOP                                                              ; @000004c0
    SUM r10, r11, r10                                                ; @000004d0  oly:15
    NOP                                                              ; @000004e0
    NOP                                                              ; @000004f0
    DLII r10, 2, r10                                                 ; @00000500  oly:15
    NOP                                                              ; @00000510
    NOP                                                              ; @00000520
    SUM r9, r10, r9                                                  ; @00000530  oly:15
    NOP                                                              ; @00000540
    NOP                                                              ; @00000550
    CP 8(r9), r9                                                     ; @00000560  oly:15
    CP -8(r30), r10                                                  ; @00000570  oly:15
    CP -48(r30), r11                                                 ; @00000580  oly:15
    CP -40(r30), r12                                                 ; @00000590  oly:15
    CP 4(r10), r13                                                   ; @000005a0  oly:15
    NOP                                                              ; @000005b0
    NOP                                                              ; @000005c0
    MUL r11, r13, r11                                                ; @000005d0  oly:15
    NOP                                                              ; @000005e0
    NOP                                                              ; @000005f0
    SUM r11, r12, r11                                                ; @00000600  oly:15
    NOP                                                              ; @00000610
    NOP                                                              ; @00000620
    DLII r11, 2, r11                                                 ; @00000630  oly:15
    NOP                                                              ; @00000640
    NOP                                                              ; @00000650
    SUM r10, r11, r10                                                ; @00000660  oly:15
    NOP                                                              ; @00000670
    NOP                                                              ; @00000680
    CP 8(r10), r10                                                   ; @00000690  oly:15
    NOP                                                              ; @000006a0
    NOP                                                              ; @000006b0
    MUL r9, r10, r9                                                  ; @000006c0  oly:15
    NOP                                                              ; @000006d0
    NOP                                                              ; @000006e0
    SUM r8, r9, r8                                                   ; @000006f0  oly:15
    NOP                                                              ; @00000700
    NOP                                                              ; @00000710
    AP -44(r30), r8                                                  ; @00000720  oly:15
    CP -48(r30), r8                                                  ; @00000730  oly:14
    NOP                                                              ; @00000740
    NOP                                                              ; @00000750
    SUMI r8, 1, r8                                                   ; @00000760  oly:14
    NOP                                                              ; @00000770
    NOP                                                              ; @00000780
    AP -48(r30), r8                                                  ; @00000790  oly:14
L_4:
    CP -48(r30), r8                                                  ; @000007a0  oly:14
    CP -16(r30), r9                                                  ; @000007b0  oly:14
    NOP                                                              ; @000007c0
    NOP                                                              ; @000007d0
    SMQ r8, r9, L_5                                                  ; @000007e0  oly:14
    NOP                                                              ; @000007f0
    NOP                                                              ; @00000800
    CP -24(r30), r8                                                  ; @00000810  oly:17
    CP -36(r30), r9                                                  ; @00000820  oly:17
    CP -40(r30), r10                                                 ; @00000830  oly:17
    CP 4(r8), r11                                                    ; @00000840  oly:17
    NOP                                                              ; @00000850
    NOP                                                              ; @00000860
    MUL r9, r11, r9                                                  ; @00000870  oly:17
    NOP                                                              ; @00000880
    NOP                                                              ; @00000890
    SUM r9, r10, r9                                                  ; @000008a0  oly:17
    NOP                                                              ; @000008b0
    NOP                                                              ; @000008c0
    DLII r9, 2, r9                                                   ; @000008d0  oly:17
    NOP                                                              ; @000008e0
    NOP                                                              ; @000008f0
    SUM r8, r9, r8 | CP -44(r30), r9                                 ; @00000900  oly:17
    NOP                                                              ; @00000910
    NOP                                                              ; @00000920
    AP 8(r8), r9                                                     ; @00000930  oly:17
    CP -40(r30), r8                                                  ; @00000940  oly:12
    NOP                                                              ; @00000950
    NOP                                                              ; @00000960
    SUMI r8, 1, r8                                                   ; @00000970  oly:12
    NOP                                                              ; @00000980
    NOP                                                              ; @00000990
    AP -40(r30), r8                                                  ; @000009a0  oly:12
L_2:
    CP -40(r30), r8                                                  ; @000009b0  oly:12
    CP -20(r30), r9                                                  ; @000009c0  oly:12
    NOP                                                              ; @000009d0
    NOP                                                              ; @000009e0
    SMQ r8, r9, L_3                                                  ; @000009f0  oly:12
    NOP                                                              ; @00000a00
    NOP                                                              ; @00000a10
    CP -36(r30), r8                                                  ; @00000a20  oly:11
    NOP                                                              ; @00000a30
    NOP                                                              ; @00000a40
    SUMI r8, 1, r8                                                   ; @00000a50  oly:11
    NOP                                                              ; @00000a60
    NOP                                                              ; @00000a70
    AP -36(r30), r8                                                  ; @00000a80  oly:11
L_0:
    CP -36(r30), r8                                                  ; @00000a90  oly:11
    CP -12(r30), r9                                                  ; @00000aa0  oly:11
    NOP                                                              ; @00000ab0
    NOP                                                              ; @00000ac0
    SMQ r8, r9, L_1                                                  ; @00000ad0  oly:11
    NOP                                                              ; @00000ae0
    NOP                                                              ; @00000af0
f_multiplicar_matrices_fin:
    SUM r30, r0, r29 | CP -28(r30), r31                              ; @00000b00  oly:11
    CP -32(r30), r27                                                 ; @00000b10  oly:11
    NOP                                                              ; @00000b20
    NOP                                                              ; @00000b30
    SUM r27, r0, r30 | SRG r31                                       ; @00000b40  oly:11
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @00000b50  oly:22
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @00000b60  oly:22
    SUMI r28, 24, r8                                                 ; @00000b70  oly:23
    SUMI r28, 48, r9                                                 ; @00000b80  oly:23
    SUMI r0, 2, r10                                                  ; @00000b90  oly:23
    SUMI r0, 2, r11 | AP -4(r29), r8                                 ; @00000ba0  oly:23
    SUMI r0, 2, r12 | AP -8(r29), r9                                 ; @00000bb0  oly:23
    SUMI r28, 0, r13 | AP -12(r29), r10                              ; @00000bc0  oly:23
    AP -16(r29), r11                                                 ; @00000bd0  oly:23
    AP -20(r29), r12                                                 ; @00000be0  oly:23
    AP -24(r29), r13 | SYE f_multiplicar_matrices                    ; @00000bf0  oly:23
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000c00  oly:23
    CP -8(r30), r27                                                  ; @00000c10  oly:23
    NOP                                                              ; @00000c20
    NOP                                                              ; @00000c30
    SUM r27, r0, r30 | SRG r31                                       ; @00000c40  oly:23
