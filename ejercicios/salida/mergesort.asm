; generado por jaf desde ejercicios/mergesort.oly
; un bundle por linea, slots 0..3 separados por '|'
; @xxxxxxxx es el byte del bundle en el .bin (cada bundle ocupa 16)
; oly:N es la linea del fuente que produjo la instruccion
_inicio:
    SUMI r0, 32, r28                                                 ; @00000000  oly:1
    SUMI r0, 512, r29                                                ; @00000010  oly:1
    SUMI r0, 8, r8                                                   ; @00000020  oly:4
    DLII r28, 11, r28                                                ; @00000030  oly:1
    DLII r29, 11, r29                                                ; @00000040  oly:1
    SUMI r0, 38, r10                                                 ; @00000050  oly:4
    SUMI r28, 0, r9                                                  ; @00000060  oly:4
    SUM r29, r0, r30                                                 ; @00000070  oly:1
    SUMI r29, -64, r29                                               ; @00000080  oly:1
    SUMI r0, 1, r8 | AP 0(r9), r8                                    ; @00000090  oly:4
    NOP                                                              ; @000000a0
    NOP                                                              ; @000000b0
    SUMI r0, 8, r8 | AP 4(r9), r8                                    ; @000000c0  oly:5,4
    SUMI r0, 27, r10 | AP 8(r9), r10                                 ; @000000d0  oly:4
    NOP                                                              ; @000000e0
    NOP                                                              ; @000000f0
    SUMI r0, 43, r10 | AP 12(r9), r10                                ; @00000100  oly:4
    NOP                                                              ; @00000110
    NOP                                                              ; @00000120
    SUMI r0, 3, r10 | AP 16(r9), r10                                 ; @00000130  oly:4
    NOP                                                              ; @00000140
    NOP                                                              ; @00000150
    SUMI r0, 9, r10 | AP 20(r9), r10                                 ; @00000160  oly:4
    NOP                                                              ; @00000170
    NOP                                                              ; @00000180
    SUMI r0, 82, r10 | AP 24(r9), r10                                ; @00000190  oly:4
    NOP                                                              ; @000001a0
    NOP                                                              ; @000001b0
    SUMI r0, 10, r10 | AP 28(r9), r10                                ; @000001c0  oly:4
    NOP                                                              ; @000001d0
    NOP                                                              ; @000001e0
    SUMI r0, 1, r10 | AP 32(r9), r10                                 ; @000001f0  oly:4
    NOP                                                              ; @00000200
    NOP                                                              ; @00000210
    SUMI r28, 40, r9 | AP 36(r9), r10                                ; @00000220  oly:5,4
    NOP                                                              ; @00000230
    NOP                                                              ; @00000240
    SUMI r0, 1, r8 | AP 0(r9), r8                                    ; @00000250  oly:5
    NOP                                                              ; @00000260
    NOP                                                              ; @00000270
    AP 4(r9), r8                                                     ; @00000280  oly:5
    AP 8(r9), r0                                                     ; @00000290  oly:5
    AP 12(r9), r0                                                    ; @000002a0  oly:5
    AP 16(r9), r0                                                    ; @000002b0  oly:5
    AP 20(r9), r0                                                    ; @000002c0  oly:5
    AP 24(r9), r0                                                    ; @000002d0  oly:5
    AP 28(r9), r0                                                    ; @000002e0  oly:5
    AP 32(r9), r0                                                    ; @000002f0  oly:5
    AP 36(r9), r0 | SYE f_main                                       ; @00000300  oly:5
    FIN                                                              ; @00000310  oly:5
f_fusionar:
    SUM r29, r0, r30 | AP -20(r29), r30                              ; @00000320  oly:7
    SUMI r29, -100, r29 | AP -16(r29), r31                           ; @00000330  oly:7
    NOP                                                              ; @00000340
    CP -4(r30), r8                                                   ; @00000350  oly:8
    NOP                                                              ; @00000360
    NOP                                                              ; @00000370
    AP -24(r30), r8                                                  ; @00000380  oly:8
    CP -8(r30), r8                                                   ; @00000390  oly:9
    NOP                                                              ; @000003a0
    NOP                                                              ; @000003b0
    SUMI r8, 1, r8                                                   ; @000003c0  oly:9
    NOP                                                              ; @000003d0
    NOP                                                              ; @000003e0
    AP -28(r30), r8                                                  ; @000003f0  oly:9
    CP -4(r30), r8                                                   ; @00000400  oly:10
    NOP                                                              ; @00000410
    NOP                                                              ; @00000420
    AP -32(r30), r8 | S L_0                                          ; @00000430  oly:10,20
L_1:
    SUMI r28, 0, r8 | CP -24(r30), r9                                ; @00000440  oly:12
    NOP                                                              ; @00000450
    NOP                                                              ; @00000460
    DLII r9, 2, r9                                                   ; @00000470  oly:12
    NOP                                                              ; @00000480
    NOP                                                              ; @00000490
    SUM r8, r9, r8                                                   ; @000004a0  oly:12
    SUMI r28, 0, r9                                                  ; @000004b0  oly:12
    NOP                                                              ; @000004c0
    CP 8(r8), r8                                                     ; @000004d0  oly:12
    CP -28(r30), r10                                                 ; @000004e0  oly:12
    NOP                                                              ; @000004f0
    NOP                                                              ; @00000500
    DLII r10, 2, r10                                                 ; @00000510  oly:12
    NOP                                                              ; @00000520
    NOP                                                              ; @00000530
    SUM r9, r10, r9                                                  ; @00000540  oly:12
    NOP                                                              ; @00000550
    NOP                                                              ; @00000560
    CP 8(r9), r9                                                     ; @00000570  oly:12
    NOP                                                              ; @00000580
    NOP                                                              ; @00000590
    SMQ r9, r8, L_2                                                  ; @000005a0  oly:12
    NOP                                                              ; @000005b0
    NOP                                                              ; @000005c0
    SUMI r28, 40, r8 | CP -32(r30), r9                               ; @000005d0  oly:13
    CP -24(r30), r10                                                 ; @000005e0  oly:13
    NOP                                                              ; @000005f0
    DLII r9, 2, r9                                                   ; @00000600  oly:13
    DLII r10, 2, r10                                                 ; @00000610  oly:13
    NOP                                                              ; @00000620
    SUM r8, r9, r8                                                   ; @00000630  oly:13
    SUMI r28, 0, r9                                                  ; @00000640  oly:13
    NOP                                                              ; @00000650
    NOP                                                              ; @00000660
    SUM r9, r10, r9                                                  ; @00000670  oly:13
    NOP                                                              ; @00000680
    NOP                                                              ; @00000690
    CP 8(r9), r9                                                     ; @000006a0  oly:13
    NOP                                                              ; @000006b0
    NOP                                                              ; @000006c0
    AP 8(r8), r9                                                     ; @000006d0  oly:13
    CP -24(r30), r8                                                  ; @000006e0  oly:14
    NOP                                                              ; @000006f0
    NOP                                                              ; @00000700
    SUMI r8, 1, r8                                                   ; @00000710  oly:14
    NOP                                                              ; @00000720
    NOP                                                              ; @00000730
    AP -24(r30), r8 | S L_3                                          ; @00000740  oly:14
L_2:
    SUMI r28, 40, r8 | CP -32(r30), r9                               ; @00000750  oly:16
    CP -28(r30), r10                                                 ; @00000760  oly:16
    NOP                                                              ; @00000770
    DLII r9, 2, r9                                                   ; @00000780  oly:16
    DLII r10, 2, r10                                                 ; @00000790  oly:16
    NOP                                                              ; @000007a0
    SUM r8, r9, r8                                                   ; @000007b0  oly:16
    SUMI r28, 0, r9                                                  ; @000007c0  oly:16
    NOP                                                              ; @000007d0
    NOP                                                              ; @000007e0
    SUM r9, r10, r9                                                  ; @000007f0  oly:16
    NOP                                                              ; @00000800
    NOP                                                              ; @00000810
    CP 8(r9), r9                                                     ; @00000820  oly:16
    NOP                                                              ; @00000830
    NOP                                                              ; @00000840
    AP 8(r8), r9                                                     ; @00000850  oly:16
    CP -28(r30), r8                                                  ; @00000860  oly:17
    NOP                                                              ; @00000870
    NOP                                                              ; @00000880
    SUMI r8, 1, r8                                                   ; @00000890  oly:17
    NOP                                                              ; @000008a0
    NOP                                                              ; @000008b0
    AP -28(r30), r8                                                  ; @000008c0  oly:17
L_3:
    CP -32(r30), r8                                                  ; @000008d0  oly:19
    NOP                                                              ; @000008e0
    NOP                                                              ; @000008f0
    SUMI r8, 1, r8                                                   ; @00000900  oly:19
    NOP                                                              ; @00000910
    NOP                                                              ; @00000920
    AP -32(r30), r8                                                  ; @00000930  oly:19
L_0:
    CP -24(r30), r8                                                  ; @00000940  oly:11
    CP -8(r30), r9                                                   ; @00000950  oly:11
    NOP                                                              ; @00000960
    NOP                                                              ; @00000970
    SMQ r9, r8, L_4                                                  ; @00000980  oly:11
    NOP                                                              ; @00000990
    NOP                                                              ; @000009a0
    CP -28(r30), r8                                                  ; @000009b0  oly:11
    CP -12(r30), r9                                                  ; @000009c0  oly:11
    NOP                                                              ; @000009d0
    NOP                                                              ; @000009e0
    SMI r9, r8, L_1                                                  ; @000009f0  oly:11
    NOP                                                              ; @00000a00
    NOP                                                              ; @00000a10
L_4:
    S L_5                                                            ; @00000a20  oly:25
L_6:
    SUMI r28, 40, r8 | CP -32(r30), r9                               ; @00000a30  oly:22
    CP -24(r30), r10                                                 ; @00000a40  oly:22
    NOP                                                              ; @00000a50
    DLII r9, 2, r9                                                   ; @00000a60  oly:22
    DLII r10, 2, r10                                                 ; @00000a70  oly:22
    NOP                                                              ; @00000a80
    SUM r8, r9, r8                                                   ; @00000a90  oly:22
    SUMI r28, 0, r9                                                  ; @00000aa0  oly:22
    NOP                                                              ; @00000ab0
    NOP                                                              ; @00000ac0
    SUM r9, r10, r9                                                  ; @00000ad0  oly:22
    NOP                                                              ; @00000ae0
    NOP                                                              ; @00000af0
    CP 8(r9), r9                                                     ; @00000b00  oly:22
    NOP                                                              ; @00000b10
    NOP                                                              ; @00000b20
    AP 8(r8), r9                                                     ; @00000b30  oly:22
    CP -24(r30), r8                                                  ; @00000b40  oly:23
    NOP                                                              ; @00000b50
    NOP                                                              ; @00000b60
    SUMI r8, 1, r8                                                   ; @00000b70  oly:23
    NOP                                                              ; @00000b80
    NOP                                                              ; @00000b90
    AP -24(r30), r8                                                  ; @00000ba0  oly:23
    CP -32(r30), r8                                                  ; @00000bb0  oly:24
    NOP                                                              ; @00000bc0
    NOP                                                              ; @00000bd0
    SUMI r8, 1, r8                                                   ; @00000be0  oly:24
    NOP                                                              ; @00000bf0
    NOP                                                              ; @00000c00
    AP -32(r30), r8                                                  ; @00000c10  oly:24
L_5:
    CP -24(r30), r8                                                  ; @00000c20  oly:21
    CP -8(r30), r9                                                   ; @00000c30  oly:21
    NOP                                                              ; @00000c40
    NOP                                                              ; @00000c50
    SMI r9, r8, L_6                                                  ; @00000c60  oly:21
    NOP                                                              ; @00000c70
    NOP                                                              ; @00000c80
    S L_7                                                            ; @00000c90  oly:30
L_8:
    SUMI r28, 40, r8 | CP -32(r30), r9                               ; @00000ca0  oly:27
    CP -28(r30), r10                                                 ; @00000cb0  oly:27
    NOP                                                              ; @00000cc0
    DLII r9, 2, r9                                                   ; @00000cd0  oly:27
    DLII r10, 2, r10                                                 ; @00000ce0  oly:27
    NOP                                                              ; @00000cf0
    SUM r8, r9, r8                                                   ; @00000d00  oly:27
    SUMI r28, 0, r9                                                  ; @00000d10  oly:27
    NOP                                                              ; @00000d20
    NOP                                                              ; @00000d30
    SUM r9, r10, r9                                                  ; @00000d40  oly:27
    NOP                                                              ; @00000d50
    NOP                                                              ; @00000d60
    CP 8(r9), r9                                                     ; @00000d70  oly:27
    NOP                                                              ; @00000d80
    NOP                                                              ; @00000d90
    AP 8(r8), r9                                                     ; @00000da0  oly:27
    CP -28(r30), r8                                                  ; @00000db0  oly:28
    NOP                                                              ; @00000dc0
    NOP                                                              ; @00000dd0
    SUMI r8, 1, r8                                                   ; @00000de0  oly:28
    NOP                                                              ; @00000df0
    NOP                                                              ; @00000e00
    AP -28(r30), r8                                                  ; @00000e10  oly:28
    CP -32(r30), r8                                                  ; @00000e20  oly:29
    NOP                                                              ; @00000e30
    NOP                                                              ; @00000e40
    SUMI r8, 1, r8                                                   ; @00000e50  oly:29
    NOP                                                              ; @00000e60
    NOP                                                              ; @00000e70
    AP -32(r30), r8                                                  ; @00000e80  oly:29
L_7:
    CP -28(r30), r8                                                  ; @00000e90  oly:26
    CP -12(r30), r9                                                  ; @00000ea0  oly:26
    NOP                                                              ; @00000eb0
    NOP                                                              ; @00000ec0
    SMI r9, r8, L_8                                                  ; @00000ed0  oly:26
    NOP                                                              ; @00000ee0
    NOP                                                              ; @00000ef0
    CP -4(r30), r8                                                   ; @00000f00  oly:31
    NOP                                                              ; @00000f10
    NOP                                                              ; @00000f20
    AP -36(r30), r8 | S L_9                                          ; @00000f30  oly:31
L_10:
    SUMI r28, 0, r8 | CP -36(r30), r9                                ; @00000f40  oly:32
    CP -36(r30), r10                                                 ; @00000f50  oly:32
    NOP                                                              ; @00000f60
    DLII r9, 2, r9                                                   ; @00000f70  oly:32
    DLII r10, 2, r10                                                 ; @00000f80  oly:32
    NOP                                                              ; @00000f90
    SUM r8, r9, r8                                                   ; @00000fa0  oly:32
    SUMI r28, 40, r9                                                 ; @00000fb0  oly:32
    NOP                                                              ; @00000fc0
    NOP                                                              ; @00000fd0
    SUM r9, r10, r9                                                  ; @00000fe0  oly:32
    NOP                                                              ; @00000ff0
    NOP                                                              ; @00001000
    CP 8(r9), r9                                                     ; @00001010  oly:32
    NOP                                                              ; @00001020
    NOP                                                              ; @00001030
    AP 8(r8), r9                                                     ; @00001040  oly:32
    CP -36(r30), r8                                                  ; @00001050  oly:31
    NOP                                                              ; @00001060
    NOP                                                              ; @00001070
    SUMI r8, 1, r8                                                   ; @00001080  oly:31
    NOP                                                              ; @00001090
    NOP                                                              ; @000010a0
    AP -36(r30), r8                                                  ; @000010b0  oly:31
L_9:
    CP -36(r30), r8                                                  ; @000010c0  oly:31
    CP -12(r30), r9                                                  ; @000010d0  oly:31
    NOP                                                              ; @000010e0
    NOP                                                              ; @000010f0
    SMI r9, r8, L_10                                                 ; @00001100  oly:31
    NOP                                                              ; @00001110
    NOP                                                              ; @00001120
f_fusionar_fin:
    SUM r30, r0, r29 | CP -16(r30), r31                              ; @00001130  oly:31
    CP -20(r30), r27                                                 ; @00001140  oly:31
    NOP                                                              ; @00001150
    NOP                                                              ; @00001160
    SUM r27, r0, r30 | SRG r31                                       ; @00001170  oly:31
f_ordenar:
    SUM r29, r0, r30 | AP -16(r29), r30                              ; @00001180  oly:36
    SUMI r29, -84, r29 | AP -12(r29), r31                            ; @00001190  oly:36
    NOP                                                              ; @000011a0
    CP -4(r30), r8                                                   ; @000011b0  oly:37
    CP -8(r30), r9                                                   ; @000011c0  oly:37
    NOP                                                              ; @000011d0
    NOP                                                              ; @000011e0
    SMI r8, r9, L_11                                                 ; @000011f0  oly:37
    NOP                                                              ; @00001200
    NOP                                                              ; @00001210
    CP -4(r30), r8                                                   ; @00001220  oly:38
    CP -8(r30), r9                                                   ; @00001230  oly:38
    NOP                                                              ; @00001240
    NOP                                                              ; @00001250
    SUM r8, r9, r8                                                   ; @00001260  oly:38
    NOP                                                              ; @00001270
    NOP                                                              ; @00001280
    DIVI r8, 2, r8                                                   ; @00001290  oly:38
    NOP                                                              ; @000012a0
    NOP                                                              ; @000012b0
    AP -20(r30), r8                                                  ; @000012c0  oly:38
    CP -4(r30), r8                                                   ; @000012d0  oly:39
    CP -20(r30), r9                                                  ; @000012e0  oly:39
    NOP                                                              ; @000012f0
    AP -4(r29), r8                                                   ; @00001300  oly:39
    AP -8(r29), r9 | SYE f_ordenar                                   ; @00001310  oly:39
    CP -20(r30), r8                                                  ; @00001320  oly:40
    CP -8(r30), r9                                                   ; @00001330  oly:40
    NOP                                                              ; @00001340
    SUMI r8, 1, r8                                                   ; @00001350  oly:40
    NOP                                                              ; @00001360
    NOP                                                              ; @00001370
    AP -4(r29), r8                                                   ; @00001380  oly:40
    AP -8(r29), r9 | SYE f_ordenar                                   ; @00001390  oly:40
    CP -4(r30), r8                                                   ; @000013a0  oly:41
    CP -20(r30), r9                                                  ; @000013b0  oly:41
    CP -8(r30), r10                                                  ; @000013c0  oly:41
    AP -4(r29), r8                                                   ; @000013d0  oly:41
    AP -8(r29), r9                                                   ; @000013e0  oly:41
    AP -12(r29), r10 | SYE f_fusionar                                ; @000013f0  oly:41
L_11:
f_ordenar_fin:
    SUM r30, r0, r29 | CP -12(r30), r31                              ; @00001400  oly:41
    CP -16(r30), r27                                                 ; @00001410  oly:41
    NOP                                                              ; @00001420
    NOP                                                              ; @00001430
    SUM r27, r0, r30 | SRG r31                                       ; @00001440  oly:41
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @00001450  oly:45
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @00001460  oly:45
    SUMI r0, 0, r8                                                   ; @00001470  oly:46
    SUMI r0, 7, r9                                                   ; @00001480  oly:46
    NOP                                                              ; @00001490
    AP -4(r29), r8                                                   ; @000014a0  oly:46
    AP -8(r29), r9 | SYE f_ordenar                                   ; @000014b0  oly:46
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @000014c0  oly:46
    CP -8(r30), r27                                                  ; @000014d0  oly:46
    NOP                                                              ; @000014e0
    NOP                                                              ; @000014f0
    SUM r27, r0, r30 | SRG r31                                       ; @00001500  oly:46
