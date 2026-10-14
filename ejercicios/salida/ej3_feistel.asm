; generado por jaf desde ejercicios/ej3_feistel.oly
; un bundle por linea, slots 0..3 separados por '|'
; @xxxxxxxx es el byte del bundle en el .bin (cada bundle ocupa 16)
; oly:N es la linea del fuente que produjo la instruccion
_inicio:
    SUMI r0, 32, r28                                                 ; @00000000  oly:1
    SUMI r0, 512, r29                                                ; @00000010  oly:1
    SUMI r0, 8, r8                                                   ; @00000020  oly:8
    DLII r28, 11, r28                                                ; @00000030  oly:1
    DLII r29, 11, r29                                                ; @00000040  oly:1
    SUMI r0, 9320, r10                                               ; @00000050  oly:8
    SUMI r28, 0, r9                                                  ; @00000060  oly:8
    SUM r29, r0, r30                                                 ; @00000070  oly:1
    SUMI r29, -64, r29                                               ; @00000080  oly:1
    SUMI r0, 1, r8 | AP 0(r9), r8                                    ; @00000090  oly:8
    DLII r10, 11, r10                                                ; @000000a0  oly:8
    NOP                                                              ; @000000b0
    SUMI r0, 8, r8 | AP 4(r9), r8                                    ; @000000c0  oly:9,8
    SUMI r10, 1383, r10                                              ; @000000d0  oly:8
    NOP                                                              ; @000000e0
    NOP                                                              ; @000000f0
    SUMI r0, 550, r10 | AP 8(r9), r10                                ; @00000100  oly:8
    NOP                                                              ; @00000110
    NOP                                                              ; @00000120
    DLII r10, 11, r10                                                ; @00000130  oly:8
    NOP                                                              ; @00000140
    NOP                                                              ; @00000150
    SUMI r10, 1401, r10                                              ; @00000160  oly:8
    NOP                                                              ; @00000170
    NOP                                                              ; @00000180
    DLII r10, 11, r10                                                ; @00000190  oly:8
    NOP                                                              ; @000001a0
    NOP                                                              ; @000001b0
    SUMI r10, 1519, r10                                              ; @000001c0  oly:8
    NOP                                                              ; @000001d0
    NOP                                                              ; @000001e0
    SUMI r0, 890, r10 | AP 12(r9), r10                               ; @000001f0  oly:8
    NOP                                                              ; @00000200
    NOP                                                              ; @00000210
    DLII r10, 11, r10                                                ; @00000220  oly:8
    NOP                                                              ; @00000230
    NOP                                                              ; @00000240
    SUMI r10, 1463, r10                                              ; @00000250  oly:8
    NOP                                                              ; @00000260
    NOP                                                              ; @00000270
    DLII r10, 11, r10                                                ; @00000280  oly:8
    NOP                                                              ; @00000290
    NOP                                                              ; @000002a0
    SUMI r10, 1775, r10                                              ; @000002b0  oly:8
    NOP                                                              ; @000002c0
    NOP                                                              ; @000002d0
    SUMI r0, 46, r10 | AP 16(r9), r10                                ; @000002e0  oly:8
    NOP                                                              ; @000002f0
    NOP                                                              ; @00000300
    DLII r10, 11, r10                                                ; @00000310  oly:8
    NOP                                                              ; @00000320
    NOP                                                              ; @00000330
    SUMI r10, 1470, r10                                              ; @00000340  oly:8
    NOP                                                              ; @00000350
    NOP                                                              ; @00000360
    DLII r10, 11, r10                                                ; @00000370  oly:8
    NOP                                                              ; @00000380
    NOP                                                              ; @00000390
    SUMI r10, 13, r10                                                ; @000003a0  oly:8
    NOP                                                              ; @000003b0
    NOP                                                              ; @000003c0
    SUMI r0, 0, r10 | AP 20(r9), r10                                 ; @000003d0  oly:8
    NOP                                                              ; @000003e0
    NOP                                                              ; @000003f0
    SUMI r0, 0, r10 | AP 24(r9), r10                                 ; @00000400  oly:8
    NOP                                                              ; @00000410
    NOP                                                              ; @00000420
    SUMI r0, -1, r10 | AP 28(r9), r10                                ; @00000430  oly:8
    NOP                                                              ; @00000440
    NOP                                                              ; @00000450
    SUMI r0, 1, r10 | AP 32(r9), r10                                 ; @00000460  oly:8
    NOP                                                              ; @00000470
    NOP                                                              ; @00000480
    SUMI r28, 40, r9 | AP 36(r9), r10                                ; @00000490  oly:9,8
    NOP                                                              ; @000004a0
    NOP                                                              ; @000004b0
    SUMI r0, 1, r8 | AP 0(r9), r8                                    ; @000004c0  oly:9
    NOP                                                              ; @000004d0
    NOP                                                              ; @000004e0
    SUMI r0, 8, r8 | AP 4(r9), r8                                    ; @000004f0  oly:10,9
    AP 8(r9), r0                                                     ; @00000500  oly:9
    AP 12(r9), r0                                                    ; @00000510  oly:9
    AP 16(r9), r0                                                    ; @00000520  oly:9
    AP 20(r9), r0                                                    ; @00000530  oly:9
    AP 24(r9), r0                                                    ; @00000540  oly:9
    AP 28(r9), r0                                                    ; @00000550  oly:9
    AP 32(r9), r0                                                    ; @00000560  oly:9
    SUMI r28, 80, r9 | AP 36(r9), r0                                 ; @00000570  oly:10,9
    NOP                                                              ; @00000580
    NOP                                                              ; @00000590
    SUMI r0, 1, r8 | AP 0(r9), r8                                    ; @000005a0  oly:10
    NOP                                                              ; @000005b0
    NOP                                                              ; @000005c0
    SUMI r0, 0, r8 | AP 4(r9), r8                                    ; @000005d0  oly:11,10
    AP 8(r9), r0                                                     ; @000005e0  oly:10
    AP 12(r9), r0                                                    ; @000005f0  oly:10
    AP 16(r9), r0                                                    ; @00000600  oly:10
    AP 20(r9), r0                                                    ; @00000610  oly:10
    AP 24(r9), r0                                                    ; @00000620  oly:10
    AP 28(r9), r0                                                    ; @00000630  oly:10
    AP 32(r9), r0                                                    ; @00000640  oly:10
    AP 36(r9), r0                                                    ; @00000650  oly:10
    SUMI r0, 0, r8 | AP 120(r28), r8                                 ; @00000660  oly:12,11
    NOP                                                              ; @00000670
    NOP                                                              ; @00000680
    SUMI r0, 0, r8 | AP 124(r28), r8                                 ; @00000690  oly:13,12
    NOP                                                              ; @000006a0
    NOP                                                              ; @000006b0
    AP 128(r28), r8 | SYE f_main                                     ; @000006c0  oly:13
    FIN                                                              ; @000006d0  oly:13
f_configurar_boveda:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @000006e0  oly:15
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @000006f0  oly:15
    SUMI r0, 811, r8                                                 ; @00000700  oly:16
    NOP                                                              ; @00000710
    NOP                                                              ; @00000720
    DLII r8, 11, r8                                                  ; @00000730  oly:16
    NOP                                                              ; @00000740
    NOP                                                              ; @00000750
    SUMI r8, 1986, r8                                                ; @00000760  oly:16
    NOP                                                              ; @00000770
    NOP                                                              ; @00000780
    DLII r8, 11, r8                                                  ; @00000790  oly:16
    NOP                                                              ; @000007a0
    NOP                                                              ; @000007b0
    SUMI r8, 564, r8                                                 ; @000007c0  oly:16
    NOP                                                              ; @000007d0
    NOP                                                              ; @000007e0
    SUMI r0, 548, r8 | AUTH r8                                       ; @000007f0  oly:17,16
    NOP                                                              ; @00000800
    NOP                                                              ; @00000810
    DLII r8, 11, r8                                                  ; @00000820  oly:17
    NOP                                                              ; @00000830
    NOP                                                              ; @00000840
    SUMI r8, 563, r8                                                 ; @00000850  oly:17
    NOP                                                              ; @00000860
    NOP                                                              ; @00000870
    SUM r0, r0, r8 | LOADKEY r8, 0, 0                                ; @00000880  oly:17
    SUMI r0, 273, r8                                                 ; @00000890  oly:18
    NOP                                                              ; @000008a0
    NOP                                                              ; @000008b0
    DLII r8, 11, r8                                                  ; @000008c0  oly:18
    NOP                                                              ; @000008d0
    NOP                                                              ; @000008e0
    SUMI r8, 684, r8                                                 ; @000008f0  oly:18
    NOP                                                              ; @00000900
    NOP                                                              ; @00000910
    DLII r8, 11, r8                                                  ; @00000920  oly:18
    NOP                                                              ; @00000930
    NOP                                                              ; @00000940
    SUMI r8, 1655, r8                                                ; @00000950  oly:18
    NOP                                                              ; @00000960
    NOP                                                              ; @00000970
    SUM r0, r0, r8 | LOADKEY r8, 0, 1                                ; @00000980  oly:18
    SUMI r0, 546, r8                                                 ; @00000990  oly:19
    NOP                                                              ; @000009a0
    NOP                                                              ; @000009b0
    DLII r8, 11, r8                                                  ; @000009c0  oly:19
    NOP                                                              ; @000009d0
    NOP                                                              ; @000009e0
    SUMI r8, 821, r8                                                 ; @000009f0  oly:19
    NOP                                                              ; @00000a00
    NOP                                                              ; @00000a10
    DLII r8, 11, r8                                                  ; @00000a20  oly:19
    NOP                                                              ; @00000a30
    NOP                                                              ; @00000a40
    SUMI r8, 699, r8                                                 ; @00000a50  oly:19
    NOP                                                              ; @00000a60
    NOP                                                              ; @00000a70
    SUM r0, r0, r8 | LOADKEY r8, 0, 2                                ; @00000a80  oly:19
    SUMI r0, 819, r8                                                 ; @00000a90  oly:20
    NOP                                                              ; @00000aa0
    NOP                                                              ; @00000ab0
    DLII r8, 11, r8                                                  ; @00000ac0  oly:20
    NOP                                                              ; @00000ad0
    NOP                                                              ; @00000ae0
    SUMI r8, 957, r8                                                 ; @00000af0  oly:20
    NOP                                                              ; @00000b00
    NOP                                                              ; @00000b10
    DLII r8, 11, r8                                                  ; @00000b20  oly:20
    NOP                                                              ; @00000b30
    NOP                                                              ; @00000b40
    SUMI r8, 1791, r8                                                ; @00000b50  oly:20
    NOP                                                              ; @00000b60
    NOP                                                              ; @00000b70
    SUM r0, r0, r8 | LOADKEY r8, 0, 3                                ; @00000b80  oly:20
    NOP                                                              ; @00000b90
    NOP                                                              ; @00000ba0
f_configurar_boveda_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000bb0  oly:20
    CP -8(r30), r27                                                  ; @00000bc0  oly:20
    NOP                                                              ; @00000bd0
    NOP                                                              ; @00000be0
    SUM r27, r0, r30 | SRG r31                                       ; @00000bf0  oly:20
f_copiar:
    SUM r29, r0, r30 | AP -20(r29), r30                              ; @00000c00  oly:23
    SUMI r29, -88, r29 | AP -16(r29), r31                            ; @00000c10  oly:23
    SUMI r0, 0, r8                                                   ; @00000c20  oly:24
    NOP                                                              ; @00000c30
    NOP                                                              ; @00000c40
    AP -24(r30), r8 | S L_0                                          ; @00000c50  oly:24
L_1:
    CP -8(r30), r8                                                   ; @00000c60  oly:25
    CP -24(r30), r9                                                  ; @00000c70  oly:25
    NOP                                                              ; @00000c80
    NOP                                                              ; @00000c90
    DLII r9, 2, r9                                                   ; @00000ca0  oly:25
    NOP                                                              ; @00000cb0
    NOP                                                              ; @00000cc0
    SUM r8, r9, r8 | CP -4(r30), r9                                  ; @00000cd0  oly:25
    CP -24(r30), r10                                                 ; @00000ce0  oly:25
    NOP                                                              ; @00000cf0
    NOP                                                              ; @00000d00
    DLII r10, 2, r10                                                 ; @00000d10  oly:25
    NOP                                                              ; @00000d20
    NOP                                                              ; @00000d30
    SUM r9, r10, r9                                                  ; @00000d40  oly:25
    NOP                                                              ; @00000d50
    NOP                                                              ; @00000d60
    CP 8(r9), r9                                                     ; @00000d70  oly:25
    NOP                                                              ; @00000d80
    NOP                                                              ; @00000d90
    AP 8(r8), r9                                                     ; @00000da0  oly:25
    CP -24(r30), r8                                                  ; @00000db0  oly:24
    NOP                                                              ; @00000dc0
    NOP                                                              ; @00000dd0
    SUMI r8, 1, r8                                                   ; @00000de0  oly:24
    NOP                                                              ; @00000df0
    NOP                                                              ; @00000e00
    AP -24(r30), r8                                                  ; @00000e10  oly:24
L_0:
    CP -24(r30), r8                                                  ; @00000e20  oly:24
    CP -12(r30), r9                                                  ; @00000e30  oly:24
    NOP                                                              ; @00000e40
    NOP                                                              ; @00000e50
    SMQ r8, r9, L_1                                                  ; @00000e60  oly:24
    NOP                                                              ; @00000e70
    NOP                                                              ; @00000e80
f_copiar_fin:
    SUM r30, r0, r29 | CP -16(r30), r31                              ; @00000e90  oly:24
    CP -20(r30), r27                                                 ; @00000ea0  oly:24
    NOP                                                              ; @00000eb0
    NOP                                                              ; @00000ec0
    SUM r27, r0, r30 | SRG r31                                       ; @00000ed0  oly:24
f_cifrar_lista:
    SUM r29, r0, r30 | AP -16(r29), r30                              ; @00000ee0  oly:29
    SUMI r29, -92, r29 | AP -12(r29), r31                            ; @00000ef0  oly:29
    SUMI r0, 0, r8                                                   ; @00000f00  oly:30
    NOP                                                              ; @00000f10
    NOP                                                              ; @00000f20
    AP -20(r30), r8 | S L_2                                          ; @00000f30  oly:30
L_3:
    SUMI r0, 2, r9 | CP -4(r30), r8                                  ; @00000f40  oly:31
    CP -20(r30), r10                                                 ; @00000f50  oly:31
    NOP                                                              ; @00000f60
    NOP                                                              ; @00000f70
    MUL r9, r10, r9                                                  ; @00000f80  oly:31
    NOP                                                              ; @00000f90
    NOP                                                              ; @00000fa0
    DLII r9, 2, r9                                                   ; @00000fb0  oly:31
    NOP                                                              ; @00000fc0
    NOP                                                              ; @00000fd0
    SUM r8, r9, r8                                                   ; @00000fe0  oly:31
    SUMI r0, 2, r9                                                   ; @00000ff0  oly:32
    NOP                                                              ; @00001000
    CP 8(r8), r8                                                     ; @00001010  oly:31
    NOP                                                              ; @00001020
    NOP                                                              ; @00001030
    AP -24(r30), r8                                                  ; @00001040  oly:31
    CP -4(r30), r8                                                   ; @00001050  oly:32
    CP -20(r30), r10                                                 ; @00001060  oly:32
    NOP                                                              ; @00001070
    NOP                                                              ; @00001080
    MUL r9, r10, r9                                                  ; @00001090  oly:32
    NOP                                                              ; @000010a0
    NOP                                                              ; @000010b0
    SUMI r9, 1, r9                                                   ; @000010c0  oly:32
    NOP                                                              ; @000010d0
    NOP                                                              ; @000010e0
    DLII r9, 2, r9                                                   ; @000010f0  oly:32
    NOP                                                              ; @00001100
    NOP                                                              ; @00001110
    SUM r8, r9, r8                                                   ; @00001120  oly:32
    NOP                                                              ; @00001130
    NOP                                                              ; @00001140
    CP 8(r8), r8                                                     ; @00001150  oly:32
    NOP                                                              ; @00001160
    NOP                                                              ; @00001170
    AP -28(r30), r8                                                  ; @00001180  oly:32
    CP -24(r30), r8                                                  ; @00001190  oly:33
    CP -28(r30), r9                                                  ; @000011a0  oly:33
    NOP                                                              ; @000011b0
    NOP                                                              ; @000011c0
    FROUND r8, r9, 0, 0, r10, r11                                    ; @000011d0  oly:33
    NOP                                                              ; @000011e0
    NOP                                                              ; @000011f0
    FROUND r10, r11, 0, 1, r8, r9                                    ; @00001200  oly:33
    NOP                                                              ; @00001210
    NOP                                                              ; @00001220
    FROUND r8, r9, 0, 2, r10, r11                                    ; @00001230  oly:33
    NOP                                                              ; @00001240
    NOP                                                              ; @00001250
    FROUND r10, r11, 0, 3, r8, r9                                    ; @00001260  oly:33
    NOP                                                              ; @00001270
    NOP                                                              ; @00001280
    AP -24(r30), r8                                                  ; @00001290  oly:33
    SUMI r0, 2, r9 | AP -28(r30), r9                                 ; @000012a0  oly:34,33
    CP -4(r30), r8                                                   ; @000012b0  oly:34
    CP -20(r30), r10                                                 ; @000012c0  oly:34
    NOP                                                              ; @000012d0
    NOP                                                              ; @000012e0
    MUL r9, r10, r9                                                  ; @000012f0  oly:34
    NOP                                                              ; @00001300
    NOP                                                              ; @00001310
    DLII r9, 2, r9                                                   ; @00001320  oly:34
    NOP                                                              ; @00001330
    NOP                                                              ; @00001340
    SUM r8, r9, r8 | CP -24(r30), r9                                 ; @00001350  oly:34
    NOP                                                              ; @00001360
    NOP                                                              ; @00001370
    SUMI r0, 2, r9 | AP 8(r8), r9                                    ; @00001380  oly:35,34
    CP -4(r30), r8                                                   ; @00001390  oly:35
    CP -20(r30), r10                                                 ; @000013a0  oly:35
    NOP                                                              ; @000013b0
    NOP                                                              ; @000013c0
    MUL r9, r10, r9                                                  ; @000013d0  oly:35
    NOP                                                              ; @000013e0
    NOP                                                              ; @000013f0
    SUMI r9, 1, r9                                                   ; @00001400  oly:35
    NOP                                                              ; @00001410
    NOP                                                              ; @00001420
    DLII r9, 2, r9                                                   ; @00001430  oly:35
    NOP                                                              ; @00001440
    NOP                                                              ; @00001450
    SUM r8, r9, r8 | CP -28(r30), r9                                 ; @00001460  oly:35
    NOP                                                              ; @00001470
    NOP                                                              ; @00001480
    AP 8(r8), r9                                                     ; @00001490  oly:35
    CP -20(r30), r8                                                  ; @000014a0  oly:30
    NOP                                                              ; @000014b0
    NOP                                                              ; @000014c0
    SUMI r8, 1, r8                                                   ; @000014d0  oly:30
    NOP                                                              ; @000014e0
    NOP                                                              ; @000014f0
    AP -20(r30), r8                                                  ; @00001500  oly:30
L_2:
    CP -20(r30), r8                                                  ; @00001510  oly:30
    CP -8(r30), r9                                                   ; @00001520  oly:30
    NOP                                                              ; @00001530
    NOP                                                              ; @00001540
    SMQ r8, r9, L_3                                                  ; @00001550  oly:30
    NOP                                                              ; @00001560
    NOP                                                              ; @00001570
f_cifrar_lista_fin:
    SUM r30, r0, r29 | CP -12(r30), r31                              ; @00001580  oly:30
    CP -16(r30), r27                                                 ; @00001590  oly:30
    NOP                                                              ; @000015a0
    NOP                                                              ; @000015b0
    SUM r27, r0, r30 | SRG r31                                       ; @000015c0  oly:30
f_descifrar_lista:
    SUM r29, r0, r30 | AP -16(r29), r30                              ; @000015d0  oly:39
    SUMI r29, -92, r29 | AP -12(r29), r31                            ; @000015e0  oly:39
    SUMI r0, 0, r8                                                   ; @000015f0  oly:40
    NOP                                                              ; @00001600
    NOP                                                              ; @00001610
    AP -20(r30), r8 | S L_4                                          ; @00001620  oly:40
L_5:
    SUMI r0, 2, r9 | CP -4(r30), r8                                  ; @00001630  oly:41
    CP -20(r30), r10                                                 ; @00001640  oly:41
    NOP                                                              ; @00001650
    NOP                                                              ; @00001660
    MUL r9, r10, r9                                                  ; @00001670  oly:41
    NOP                                                              ; @00001680
    NOP                                                              ; @00001690
    DLII r9, 2, r9                                                   ; @000016a0  oly:41
    NOP                                                              ; @000016b0
    NOP                                                              ; @000016c0
    SUM r8, r9, r8                                                   ; @000016d0  oly:41
    SUMI r0, 2, r9                                                   ; @000016e0  oly:42
    NOP                                                              ; @000016f0
    CP 8(r8), r8                                                     ; @00001700  oly:41
    NOP                                                              ; @00001710
    NOP                                                              ; @00001720
    AP -24(r30), r8                                                  ; @00001730  oly:41
    CP -4(r30), r8                                                   ; @00001740  oly:42
    CP -20(r30), r10                                                 ; @00001750  oly:42
    NOP                                                              ; @00001760
    NOP                                                              ; @00001770
    MUL r9, r10, r9                                                  ; @00001780  oly:42
    NOP                                                              ; @00001790
    NOP                                                              ; @000017a0
    SUMI r9, 1, r9                                                   ; @000017b0  oly:42
    NOP                                                              ; @000017c0
    NOP                                                              ; @000017d0
    DLII r9, 2, r9                                                   ; @000017e0  oly:42
    NOP                                                              ; @000017f0
    NOP                                                              ; @00001800
    SUM r8, r9, r8                                                   ; @00001810  oly:42
    NOP                                                              ; @00001820
    NOP                                                              ; @00001830
    CP 8(r8), r8                                                     ; @00001840  oly:42
    NOP                                                              ; @00001850
    NOP                                                              ; @00001860
    AP -28(r30), r8                                                  ; @00001870  oly:42
    CP -24(r30), r8                                                  ; @00001880  oly:43
    CP -28(r30), r9                                                  ; @00001890  oly:43
    NOP                                                              ; @000018a0
    NOP                                                              ; @000018b0
    FROUND r9, r8, 0, 3, r10, r11                                    ; @000018c0  oly:43
    NOP                                                              ; @000018d0
    NOP                                                              ; @000018e0
    FROUND r10, r11, 0, 2, r9, r8                                    ; @000018f0  oly:43
    NOP                                                              ; @00001900
    NOP                                                              ; @00001910
    FROUND r9, r8, 0, 1, r10, r11                                    ; @00001920  oly:43
    NOP                                                              ; @00001930
    NOP                                                              ; @00001940
    FROUND r10, r11, 0, 0, r9, r8                                    ; @00001950  oly:43
    NOP                                                              ; @00001960
    NOP                                                              ; @00001970
    AP -24(r30), r8                                                  ; @00001980  oly:43
    SUMI r0, 2, r9 | AP -28(r30), r9                                 ; @00001990  oly:44,43
    CP -4(r30), r8                                                   ; @000019a0  oly:44
    CP -20(r30), r10                                                 ; @000019b0  oly:44
    NOP                                                              ; @000019c0
    NOP                                                              ; @000019d0
    MUL r9, r10, r9                                                  ; @000019e0  oly:44
    NOP                                                              ; @000019f0
    NOP                                                              ; @00001a00
    DLII r9, 2, r9                                                   ; @00001a10  oly:44
    NOP                                                              ; @00001a20
    NOP                                                              ; @00001a30
    SUM r8, r9, r8 | CP -24(r30), r9                                 ; @00001a40  oly:44
    NOP                                                              ; @00001a50
    NOP                                                              ; @00001a60
    SUMI r0, 2, r9 | AP 8(r8), r9                                    ; @00001a70  oly:45,44
    CP -4(r30), r8                                                   ; @00001a80  oly:45
    CP -20(r30), r10                                                 ; @00001a90  oly:45
    NOP                                                              ; @00001aa0
    NOP                                                              ; @00001ab0
    MUL r9, r10, r9                                                  ; @00001ac0  oly:45
    NOP                                                              ; @00001ad0
    NOP                                                              ; @00001ae0
    SUMI r9, 1, r9                                                   ; @00001af0  oly:45
    NOP                                                              ; @00001b00
    NOP                                                              ; @00001b10
    DLII r9, 2, r9                                                   ; @00001b20  oly:45
    NOP                                                              ; @00001b30
    NOP                                                              ; @00001b40
    SUM r8, r9, r8 | CP -28(r30), r9                                 ; @00001b50  oly:45
    NOP                                                              ; @00001b60
    NOP                                                              ; @00001b70
    AP 8(r8), r9                                                     ; @00001b80  oly:45
    CP -20(r30), r8                                                  ; @00001b90  oly:40
    NOP                                                              ; @00001ba0
    NOP                                                              ; @00001bb0
    SUMI r8, 1, r8                                                   ; @00001bc0  oly:40
    NOP                                                              ; @00001bd0
    NOP                                                              ; @00001be0
    AP -20(r30), r8                                                  ; @00001bf0  oly:40
L_4:
    CP -20(r30), r8                                                  ; @00001c00  oly:40
    CP -8(r30), r9                                                   ; @00001c10  oly:40
    NOP                                                              ; @00001c20
    NOP                                                              ; @00001c30
    SMQ r8, r9, L_5                                                  ; @00001c40  oly:40
    NOP                                                              ; @00001c50
    NOP                                                              ; @00001c60
f_descifrar_lista_fin:
    SUM r30, r0, r29 | CP -12(r30), r31                              ; @00001c70  oly:40
    CP -16(r30), r27                                                 ; @00001c80  oly:40
    NOP                                                              ; @00001c90
    NOP                                                              ; @00001ca0
    SUM r27, r0, r30 | SRG r31                                       ; @00001cb0  oly:40
f_iguales:
    SUM r29, r0, r30 | AP -20(r29), r30                              ; @00001cc0  oly:49
    SUMI r29, -88, r29 | AP -16(r29), r31                            ; @00001cd0  oly:49
    SUMI r0, 0, r8                                                   ; @00001ce0  oly:50
    NOP                                                              ; @00001cf0
    NOP                                                              ; @00001d00
    AP -24(r30), r8 | S L_6                                          ; @00001d10  oly:50
L_7:
    CP -4(r30), r8                                                   ; @00001d20  oly:51
    CP -24(r30), r9                                                  ; @00001d30  oly:51
    NOP                                                              ; @00001d40
    NOP                                                              ; @00001d50
    DLII r9, 2, r9                                                   ; @00001d60  oly:51
    NOP                                                              ; @00001d70
    NOP                                                              ; @00001d80
    SUM r8, r9, r8                                                   ; @00001d90  oly:51
    NOP                                                              ; @00001da0
    NOP                                                              ; @00001db0
    CP 8(r8), r8                                                     ; @00001dc0  oly:51
    CP -8(r30), r9                                                   ; @00001dd0  oly:51
    CP -24(r30), r10                                                 ; @00001de0  oly:51
    NOP                                                              ; @00001df0
    NOP                                                              ; @00001e00
    DLII r10, 2, r10                                                 ; @00001e10  oly:51
    NOP                                                              ; @00001e20
    NOP                                                              ; @00001e30
    SUM r9, r10, r9                                                  ; @00001e40  oly:51
    NOP                                                              ; @00001e50
    NOP                                                              ; @00001e60
    CP 8(r9), r9                                                     ; @00001e70  oly:51
    NOP                                                              ; @00001e80
    NOP                                                              ; @00001e90
    SIG r8, r9, L_8                                                  ; @00001ea0  oly:51
    NOP                                                              ; @00001eb0
    NOP                                                              ; @00001ec0
    SUMI r0, 0, r8                                                   ; @00001ed0  oly:52
    NOP                                                              ; @00001ee0
    NOP                                                              ; @00001ef0
    SUM r8, r0, r1 | S f_iguales_fin                                 ; @00001f00  oly:52
L_8:
    CP -24(r30), r8                                                  ; @00001f10  oly:50
    NOP                                                              ; @00001f20
    NOP                                                              ; @00001f30
    SUMI r8, 1, r8                                                   ; @00001f40  oly:50
    NOP                                                              ; @00001f50
    NOP                                                              ; @00001f60
    AP -24(r30), r8                                                  ; @00001f70  oly:50
L_6:
    CP -24(r30), r8                                                  ; @00001f80  oly:50
    CP -12(r30), r9                                                  ; @00001f90  oly:50
    NOP                                                              ; @00001fa0
    NOP                                                              ; @00001fb0
    SMQ r8, r9, L_7                                                  ; @00001fc0  oly:50
    NOP                                                              ; @00001fd0
    NOP                                                              ; @00001fe0
    SUMI r0, 1, r8                                                   ; @00001ff0  oly:55
    NOP                                                              ; @00002000
    NOP                                                              ; @00002010
    SUM r8, r0, r1                                                   ; @00002020  oly:55
    NOP                                                              ; @00002030
    NOP                                                              ; @00002040
f_iguales_fin:
    SUM r30, r0, r29 | CP -16(r30), r31                              ; @00002050  oly:55
    CP -20(r30), r27                                                 ; @00002060  oly:55
    NOP                                                              ; @00002070
    NOP                                                              ; @00002080
    SUM r27, r0, r30 | SRG r31                                       ; @00002090  oly:55
f_suma_de_control:
    SUM r29, r0, r30 | AP -16(r29), r30                              ; @000020a0  oly:58
    SUMI r29, -88, r29 | AP -12(r29), r31                            ; @000020b0  oly:58
    SUMI r0, 0, r8                                                   ; @000020c0  oly:59
    NOP                                                              ; @000020d0
    NOP                                                              ; @000020e0
    SUMI r0, 0, r8 | AP -20(r30), r8                                 ; @000020f0  oly:60,59
    NOP                                                              ; @00002100
    NOP                                                              ; @00002110
    AP -24(r30), r8 | S L_9                                          ; @00002120  oly:60
L_10:
    CP -20(r30), r8                                                  ; @00002130  oly:61
    CP -4(r30), r9                                                   ; @00002140  oly:61
    CP -24(r30), r10                                                 ; @00002150  oly:61
    NOP                                                              ; @00002160
    NOP                                                              ; @00002170
    DLII r10, 2, r10                                                 ; @00002180  oly:61
    NOP                                                              ; @00002190
    NOP                                                              ; @000021a0
    SUM r9, r10, r9                                                  ; @000021b0  oly:61
    NOP                                                              ; @000021c0
    NOP                                                              ; @000021d0
    CP 8(r9), r9                                                     ; @000021e0  oly:61
    NOP                                                              ; @000021f0
    NOP                                                              ; @00002200
    SUM r8, r9, r8                                                   ; @00002210  oly:61
    NOP                                                              ; @00002220
    NOP                                                              ; @00002230
    AP -20(r30), r8                                                  ; @00002240  oly:61
    CP -24(r30), r8                                                  ; @00002250  oly:60
    NOP                                                              ; @00002260
    NOP                                                              ; @00002270
    SUMI r8, 1, r8                                                   ; @00002280  oly:60
    NOP                                                              ; @00002290
    NOP                                                              ; @000022a0
    AP -24(r30), r8                                                  ; @000022b0  oly:60
L_9:
    CP -24(r30), r8                                                  ; @000022c0  oly:60
    CP -8(r30), r9                                                   ; @000022d0  oly:60
    NOP                                                              ; @000022e0
    NOP                                                              ; @000022f0
    SMQ r8, r9, L_10                                                 ; @00002300  oly:60
    NOP                                                              ; @00002310
    NOP                                                              ; @00002320
    CP -20(r30), r8                                                  ; @00002330  oly:63
    NOP                                                              ; @00002340
    NOP                                                              ; @00002350
    SUM r8, r0, r1                                                   ; @00002360  oly:63
    NOP                                                              ; @00002370
    NOP                                                              ; @00002380
f_suma_de_control_fin:
    SUM r30, r0, r29 | CP -12(r30), r31                              ; @00002390  oly:63
    CP -16(r30), r27                                                 ; @000023a0  oly:63
    NOP                                                              ; @000023b0
    NOP                                                              ; @000023c0
    SUM r27, r0, r30 | SRG r31                                       ; @000023d0  oly:63
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @000023e0  oly:66
    SUMI r29, -72, r29 | AP -4(r29), r31 | SYE f_configurar_boveda   ; @000023f0  oly:66,67
    SUMI r28, 0, r8                                                  ; @00002400  oly:68
    SUMI r28, 40, r9                                                 ; @00002410  oly:68
    SUMI r0, 8, r10                                                  ; @00002420  oly:68
    AP -4(r29), r8                                                   ; @00002430  oly:68
    AP -8(r29), r9                                                   ; @00002440  oly:68
    AP -12(r29), r10 | SYE f_copiar                                  ; @00002450  oly:68
    SUMI r28, 0, r8                                                  ; @00002460  oly:69
    SUMI r0, 4, r9                                                   ; @00002470  oly:69
    NOP                                                              ; @00002480
    AP -4(r29), r8                                                   ; @00002490  oly:69
    AP -8(r29), r9 | SYE f_cifrar_lista                              ; @000024a0  oly:69
    SUMI r28, 0, r8                                                  ; @000024b0  oly:70
    SUMI r28, 80, r9                                                 ; @000024c0  oly:70
    SUMI r0, 8, r10                                                  ; @000024d0  oly:70
    AP -4(r29), r8                                                   ; @000024e0  oly:70
    AP -8(r29), r9                                                   ; @000024f0  oly:70
    AP -12(r29), r10 | SYE f_copiar                                  ; @00002500  oly:70
    SUMI r28, 80, r8                                                 ; @00002510  oly:71
    SUMI r0, 8, r9                                                   ; @00002520  oly:71
    NOP                                                              ; @00002530
    AP -4(r29), r8                                                   ; @00002540  oly:71
    AP -8(r29), r9 | SYE f_suma_de_control                           ; @00002550  oly:71
    SUM r1, r0, r8                                                   ; @00002560  oly:71
    SUMI r0, 4, r9                                                   ; @00002570  oly:72
    NOP                                                              ; @00002580
    SUMI r28, 0, r8 | AP 124(r28), r8                                ; @00002590  oly:72,71
    NOP                                                              ; @000025a0
    NOP                                                              ; @000025b0
    AP -4(r29), r8                                                   ; @000025c0  oly:72
    AP -8(r29), r9 | SYE f_descifrar_lista                           ; @000025d0  oly:72
    SUMI r28, 0, r8                                                  ; @000025e0  oly:73
    SUMI r28, 40, r9                                                 ; @000025f0  oly:73
    SUMI r0, 8, r10                                                  ; @00002600  oly:73
    AP -4(r29), r8                                                   ; @00002610  oly:73
    AP -8(r29), r9                                                   ; @00002620  oly:73
    AP -12(r29), r10 | SYE f_iguales                                 ; @00002630  oly:73
    SUM r1, r0, r8                                                   ; @00002640  oly:73
    NOP                                                              ; @00002650
    NOP                                                              ; @00002660
    AP 120(r28), r8 | RDSR r8                                        ; @00002670  oly:73,74
    LOGOUT                                                           ; @00002680  oly:75
    NOP                                                              ; @00002690
    AP 128(r28), r8                                                  ; @000026a0  oly:74
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @000026b0  oly:75
    CP -8(r30), r27                                                  ; @000026c0  oly:75
    NOP                                                              ; @000026d0
    NOP                                                              ; @000026e0
    SUM r27, r0, r30 | SRG r31                                       ; @000026f0  oly:75
