; generado por jaf desde ejercicios/suma_mayores.oly
; un bundle por linea, slots 0..3 separados por '|'
; @xxxxxxxx es el byte del bundle en el .bin (cada bundle ocupa 16)
; oly:N es la linea del fuente que produjo la instruccion
_inicio:
    SUMI r0, 32, r28                                                 ; @00000000  oly:1
    SUMI r0, 512, r29                                                ; @00000010  oly:1
    SUMI r0, 0, r8                                                   ; @00000020  oly:7
    DLII r28, 11, r28                                                ; @00000030  oly:1
    DLII r29, 11, r29                                                ; @00000040  oly:1
    SUMI r0, 10, r10                                                 ; @00000050  oly:9
    SUMI r0, 1, r8 | AP 0(r28), r8                                   ; @00000060  oly:8,7
    SUM r29, r0, r30                                                 ; @00000070  oly:1
    SUMI r29, -64, r29                                               ; @00000080  oly:1
    SUMI r28, 8, r9 | AP 4(r28), r8                                  ; @00000090  oly:9,8
    SUMI r0, 4, r8                                                   ; @000000a0  oly:9
    NOP                                                              ; @000000b0
    NOP                                                              ; @000000c0
    SUMI r0, 1, r8 | AP 0(r9), r8                                    ; @000000d0  oly:9
    NOP                                                              ; @000000e0
    NOP                                                              ; @000000f0
    AP 4(r9), r8                                                     ; @00000100  oly:9
    SUMI r0, 2, r10 | AP 8(r9), r10                                  ; @00000110  oly:9
    NOP                                                              ; @00000120
    NOP                                                              ; @00000130
    SUMI r0, 3, r10 | AP 12(r9), r10                                 ; @00000140  oly:9
    NOP                                                              ; @00000150
    NOP                                                              ; @00000160
    SUMI r0, 4, r10 | AP 16(r9), r10                                 ; @00000170  oly:9
    NOP                                                              ; @00000180
    NOP                                                              ; @00000190
    AP 20(r9), r10 | SYE f_main                                      ; @000001a0  oly:9
    FIN                                                              ; @000001b0  oly:9
f_maximo_lista:
    SUM r29, r0, r30 | AP -16(r29), r30                              ; @000001c0  oly:11
    SUMI r29, -88, r29 | AP -12(r29), r31                            ; @000001d0  oly:11
    SUMI r0, 0, r9                                                   ; @000001e0  oly:12
    CP -4(r30), r8                                                   ; @000001f0  oly:12
    NOP                                                              ; @00000200
    DLII r9, 2, r9                                                   ; @00000210  oly:12
    NOP                                                              ; @00000220
    NOP                                                              ; @00000230
    SUM r8, r9, r8                                                   ; @00000240  oly:12
    NOP                                                              ; @00000250
    NOP                                                              ; @00000260
    CP 8(r8), r8                                                     ; @00000270  oly:12
    NOP                                                              ; @00000280
    NOP                                                              ; @00000290
    SUMI r0, 0, r8 | AP -20(r30), r8                                 ; @000002a0  oly:13,12
    NOP                                                              ; @000002b0
    NOP                                                              ; @000002c0
    AP -24(r30), r8 | S L_0                                          ; @000002d0  oly:13
L_1:
    CP -4(r30), r8                                                   ; @000002e0  oly:14
    CP -24(r30), r9                                                  ; @000002f0  oly:14
    NOP                                                              ; @00000300
    NOP                                                              ; @00000310
    DLII r9, 2, r9                                                   ; @00000320  oly:14
    NOP                                                              ; @00000330
    NOP                                                              ; @00000340
    SUM r8, r9, r8                                                   ; @00000350  oly:14
    NOP                                                              ; @00000360
    NOP                                                              ; @00000370
    CP 8(r8), r8                                                     ; @00000380  oly:14
    CP -20(r30), r9                                                  ; @00000390  oly:14
    NOP                                                              ; @000003a0
    NOP                                                              ; @000003b0
    SMI r9, r8, L_2                                                  ; @000003c0  oly:14
    NOP                                                              ; @000003d0
    NOP                                                              ; @000003e0
    CP -4(r30), r8                                                   ; @000003f0  oly:15
    CP -24(r30), r9                                                  ; @00000400  oly:15
    NOP                                                              ; @00000410
    NOP                                                              ; @00000420
    DLII r9, 2, r9                                                   ; @00000430  oly:15
    NOP                                                              ; @00000440
    NOP                                                              ; @00000450
    SUM r8, r9, r8                                                   ; @00000460  oly:15
    NOP                                                              ; @00000470
    NOP                                                              ; @00000480
    CP 8(r8), r8                                                     ; @00000490  oly:15
    NOP                                                              ; @000004a0
    NOP                                                              ; @000004b0
    AP -20(r30), r8                                                  ; @000004c0  oly:15
L_2:
    CP -24(r30), r8                                                  ; @000004d0  oly:13
    NOP                                                              ; @000004e0
    NOP                                                              ; @000004f0
    SUMI r8, 1, r8                                                   ; @00000500  oly:13
    NOP                                                              ; @00000510
    NOP                                                              ; @00000520
    AP -24(r30), r8                                                  ; @00000530  oly:13
L_0:
    CP -24(r30), r8                                                  ; @00000540  oly:13
    CP -8(r30), r9                                                   ; @00000550  oly:13
    NOP                                                              ; @00000560
    NOP                                                              ; @00000570
    SMQ r8, r9, L_1                                                  ; @00000580  oly:13
    NOP                                                              ; @00000590
    NOP                                                              ; @000005a0
    CP -20(r30), r8                                                  ; @000005b0  oly:18
    NOP                                                              ; @000005c0
    NOP                                                              ; @000005d0
    SUM r8, r0, r1                                                   ; @000005e0  oly:18
    NOP                                                              ; @000005f0
    NOP                                                              ; @00000600
f_maximo_lista_fin:
    SUM r30, r0, r29 | CP -12(r30), r31                              ; @00000610  oly:18
    CP -16(r30), r27                                                 ; @00000620  oly:18
    NOP                                                              ; @00000630
    NOP                                                              ; @00000640
    SUM r27, r0, r30 | SRG r31                                       ; @00000650  oly:18
f_sumeMayores:
    SUM r29, r0, r30 | AP -16(r29), r30                              ; @00000660  oly:21
    SUMI r29, -84, r29 | AP -12(r29), r31 | S L_3                    ; @00000670  oly:21,34
L_4:
    CP -4(r30), r8                                                   ; @00000680  oly:23
    CP -8(r30), r9                                                   ; @00000690  oly:23
    NOP                                                              ; @000006a0
    AP -4(r29), r8                                                   ; @000006b0  oly:23
    AP -8(r29), r9 | SYE f_maximo_lista                              ; @000006c0  oly:23
    SUM r1, r0, r8                                                   ; @000006d0  oly:23
    SUMI r0, 5, r9                                                   ; @000006e0  oly:24
    NOP                                                              ; @000006f0
    AP -20(r30), r8                                                  ; @00000700  oly:23
    CP -20(r30), r8                                                  ; @00000710  oly:24
    NOP                                                              ; @00000720
    NOP                                                              ; @00000730
    DIVI r8, 2, r8                                                   ; @00000740  oly:24
    NOP                                                              ; @00000750
    NOP                                                              ; @00000760
    SNIG r8, r9, L_5                                                 ; @00000770  oly:24
    NOP                                                              ; @00000780
    NOP                                                              ; @00000790
    CP -20(r30), r8                                                  ; @000007a0  oly:25
    NOP                                                              ; @000007b0
    NOP                                                              ; @000007c0
    MULI r8, 2, r8                                                   ; @000007d0  oly:25
    NOP                                                              ; @000007e0
    NOP                                                              ; @000007f0
    AP -20(r30), r8                                                  ; @00000800  oly:25
L_5:
    CP 0(r28), r8                                                    ; @00000810  oly:27
    CP -20(r30), r9                                                  ; @00000820  oly:27
    NOP                                                              ; @00000830
    NOP                                                              ; @00000840
    SUM r8, r9, r8                                                   ; @00000850  oly:27
    NOP                                                              ; @00000860
    NOP                                                              ; @00000870
    AP 0(r28), r8                                                    ; @00000880  oly:27
    CP 4(r28), r8                                                    ; @00000890  oly:28
    CP -20(r30), r9                                                  ; @000008a0  oly:28
    NOP                                                              ; @000008b0
    NOP                                                              ; @000008c0
    MUL r8, r9, r8                                                   ; @000008d0  oly:28
    SUMI r0, 500, r9                                                 ; @000008e0  oly:29
    NOP                                                              ; @000008f0
    AP 4(r28), r8                                                    ; @00000900  oly:28
    CP 4(r28), r8                                                    ; @00000910  oly:29
    NOP                                                              ; @00000920
    NOP                                                              ; @00000930
    SMI r9, r8, L_6                                                  ; @00000940  oly:29
    NOP                                                              ; @00000950
    NOP                                                              ; @00000960
    SUMI r0, 10, r8                                                  ; @00000970  oly:30
    NOP                                                              ; @00000980
    NOP                                                              ; @00000990
    AP 4(r28), r8 | S L_7                                            ; @000009a0  oly:30
L_6:
    CP 4(r28), r8                                                    ; @000009b0  oly:32
    NOP                                                              ; @000009c0
    NOP                                                              ; @000009d0
    RESTI r8, 10, r8                                                 ; @000009e0  oly:32
    NOP                                                              ; @000009f0
    NOP                                                              ; @00000a00
    AP 4(r28), r8                                                    ; @00000a10  oly:32
L_7:
L_3:
    SUMI r0, 100, r9 | CP 0(r28), r8                                 ; @00000a20  oly:22
    NOP                                                              ; @00000a30
    NOP                                                              ; @00000a40
    SMQ r8, r9, L_4                                                  ; @00000a50  oly:22
    NOP                                                              ; @00000a60
    NOP                                                              ; @00000a70
f_sumeMayores_fin:
    SUM r30, r0, r29 | CP -12(r30), r31                              ; @00000a80  oly:22
    CP -16(r30), r27                                                 ; @00000a90  oly:22
    NOP                                                              ; @00000aa0
    NOP                                                              ; @00000ab0
    SUM r27, r0, r30 | SRG r31                                       ; @00000ac0  oly:22
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @00000ad0  oly:37
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @00000ae0  oly:37
    SUMI r28, 8, r8                                                  ; @00000af0  oly:38
    SUMI r0, 4, r9                                                   ; @00000b00  oly:38
    NOP                                                              ; @00000b10
    AP -4(r29), r8                                                   ; @00000b20  oly:38
    AP -8(r29), r9 | SYE f_sumeMayores                               ; @00000b30  oly:38
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000b40  oly:38
    CP -8(r30), r27                                                  ; @00000b50  oly:38
    NOP                                                              ; @00000b60
    NOP                                                              ; @00000b70
    SUM r27, r0, r30 | SRG r31                                       ; @00000b80  oly:38
