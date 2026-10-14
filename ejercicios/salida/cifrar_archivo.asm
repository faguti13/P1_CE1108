; generado por jaf desde ejercicios/cifrar_archivo.oly
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
f_preparar:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @000000b0  oly:8
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @000000c0  oly:8
    SUMI r0, 811, r8                                                 ; @000000d0  oly:9
    NOP                                                              ; @000000e0
    NOP                                                              ; @000000f0
    DLII r8, 11, r8                                                  ; @00000100  oly:9
    NOP                                                              ; @00000110
    NOP                                                              ; @00000120
    SUMI r8, 1986, r8                                                ; @00000130  oly:9
    NOP                                                              ; @00000140
    NOP                                                              ; @00000150
    DLII r8, 11, r8                                                  ; @00000160  oly:9
    NOP                                                              ; @00000170
    NOP                                                              ; @00000180
    SUMI r8, 564, r8                                                 ; @00000190  oly:9
    NOP                                                              ; @000001a0
    NOP                                                              ; @000001b0
    SUMI r0, 548, r8 | AUTH r8                                       ; @000001c0  oly:10,9
    NOP                                                              ; @000001d0
    NOP                                                              ; @000001e0
    DLII r8, 11, r8                                                  ; @000001f0  oly:10
    NOP                                                              ; @00000200
    NOP                                                              ; @00000210
    SUMI r8, 563, r8                                                 ; @00000220  oly:10
    NOP                                                              ; @00000230
    NOP                                                              ; @00000240
    SUM r0, r0, r8 | LOADKEY r8, 0, 0                                ; @00000250  oly:10
    SUMI r0, 273, r8                                                 ; @00000260  oly:11
    NOP                                                              ; @00000270
    NOP                                                              ; @00000280
    DLII r8, 11, r8                                                  ; @00000290  oly:11
    NOP                                                              ; @000002a0
    NOP                                                              ; @000002b0
    SUMI r8, 684, r8                                                 ; @000002c0  oly:11
    NOP                                                              ; @000002d0
    NOP                                                              ; @000002e0
    DLII r8, 11, r8                                                  ; @000002f0  oly:11
    NOP                                                              ; @00000300
    NOP                                                              ; @00000310
    SUMI r8, 1655, r8                                                ; @00000320  oly:11
    NOP                                                              ; @00000330
    NOP                                                              ; @00000340
    SUM r0, r0, r8 | LOADKEY r8, 0, 1                                ; @00000350  oly:11
    SUMI r0, 546, r8                                                 ; @00000360  oly:12
    NOP                                                              ; @00000370
    NOP                                                              ; @00000380
    DLII r8, 11, r8                                                  ; @00000390  oly:12
    NOP                                                              ; @000003a0
    NOP                                                              ; @000003b0
    SUMI r8, 821, r8                                                 ; @000003c0  oly:12
    NOP                                                              ; @000003d0
    NOP                                                              ; @000003e0
    DLII r8, 11, r8                                                  ; @000003f0  oly:12
    NOP                                                              ; @00000400
    NOP                                                              ; @00000410
    SUMI r8, 699, r8                                                 ; @00000420  oly:12
    NOP                                                              ; @00000430
    NOP                                                              ; @00000440
    SUM r0, r0, r8 | LOADKEY r8, 0, 2                                ; @00000450  oly:12
    SUMI r0, 819, r8                                                 ; @00000460  oly:13
    NOP                                                              ; @00000470
    NOP                                                              ; @00000480
    DLII r8, 11, r8                                                  ; @00000490  oly:13
    NOP                                                              ; @000004a0
    NOP                                                              ; @000004b0
    SUMI r8, 957, r8                                                 ; @000004c0  oly:13
    NOP                                                              ; @000004d0
    NOP                                                              ; @000004e0
    DLII r8, 11, r8                                                  ; @000004f0  oly:13
    NOP                                                              ; @00000500
    NOP                                                              ; @00000510
    SUMI r8, 1791, r8                                                ; @00000520  oly:13
    NOP                                                              ; @00000530
    NOP                                                              ; @00000540
    SUM r0, r0, r8 | LOADKEY r8, 0, 3                                ; @00000550  oly:13
    NOP                                                              ; @00000560
    NOP                                                              ; @00000570
f_preparar_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000580  oly:13
    CP -8(r30), r27                                                  ; @00000590  oly:13
    NOP                                                              ; @000005a0
    NOP                                                              ; @000005b0
    SUM r27, r0, r30 | SRG r31                                       ; @000005c0  oly:13
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @000005d0  oly:16
    SUMI r29, -84, r29 | AP -4(r29), r31                             ; @000005e0  oly:16
    SUMI r0, 0, r8                                                   ; @000005f0  oly:17
    NOP                                                              ; @00000600
    NOP                                                              ; @00000610
    SUMI r0, 0, r8 | AP -12(r30), r8                                 ; @00000620  oly:18,17
    NOP                                                              ; @00000630
    NOP                                                              ; @00000640
    AP -16(r30), r8 | SYE f_preparar                                 ; @00000650  oly:18,19
    SUMI r0, 63, r8                                                  ; @00000660  oly:20
    NOP                                                              ; @00000670
    NOP                                                              ; @00000680
    DLII r8, 11, r8                                                  ; @00000690  oly:20
    NOP                                                              ; @000006a0
    NOP                                                              ; @000006b0
    SUMI r8, 2032, r8                                                ; @000006c0  oly:20
    NOP                                                              ; @000006d0
    NOP                                                              ; @000006e0
    CP 0(r8), r8                                                     ; @000006f0  oly:20
    NOP                                                              ; @00000700
    NOP                                                              ; @00000710
    SUMI r0, 63, r8 | AP 0(r28), r8                                  ; @00000720  oly:21,20
    NOP                                                              ; @00000730
    NOP                                                              ; @00000740
    DLII r8, 11, r8                                                  ; @00000750  oly:21
    NOP                                                              ; @00000760
    NOP                                                              ; @00000770
    SUMI r8, 2040, r8                                                ; @00000780  oly:21
    NOP                                                              ; @00000790
    NOP                                                              ; @000007a0
    CP 0(r8), r8                                                     ; @000007b0  oly:21
    NOP                                                              ; @000007c0
    NOP                                                              ; @000007d0
    SUMI r0, 0, r8 | AP 4(r28), r8                                   ; @000007e0  oly:22,21
    NOP                                                              ; @000007f0
    NOP                                                              ; @00000800
    AP -20(r30), r8 | S L_0                                          ; @00000810  oly:22
L_1:
    SUMI r0, 8, r9 | CP 0(r28), r8                                   ; @00000820  oly:23
    CP -20(r30), r10                                                 ; @00000830  oly:23
    NOP                                                              ; @00000840
    NOP                                                              ; @00000850
    MUL r9, r10, r9                                                  ; @00000860  oly:23
    NOP                                                              ; @00000870
    NOP                                                              ; @00000880
    SUM r8, r9, r8                                                   ; @00000890  oly:23
    SUMI r0, 8, r9                                                   ; @000008a0  oly:24
    NOP                                                              ; @000008b0
    CP 0(r8), r8                                                     ; @000008c0  oly:23
    NOP                                                              ; @000008d0
    NOP                                                              ; @000008e0
    AP -12(r30), r8                                                  ; @000008f0  oly:23
    CP 0(r28), r8                                                    ; @00000900  oly:24
    CP -20(r30), r10                                                 ; @00000910  oly:24
    NOP                                                              ; @00000920
    NOP                                                              ; @00000930
    MUL r9, r10, r9                                                  ; @00000940  oly:24
    NOP                                                              ; @00000950
    NOP                                                              ; @00000960
    SUM r8, r9, r8                                                   ; @00000970  oly:24
    NOP                                                              ; @00000980
    NOP                                                              ; @00000990
    SUMI r8, 4, r8                                                   ; @000009a0  oly:24
    NOP                                                              ; @000009b0
    NOP                                                              ; @000009c0
    CP 0(r8), r8                                                     ; @000009d0  oly:24
    NOP                                                              ; @000009e0
    NOP                                                              ; @000009f0
    AP -16(r30), r8                                                  ; @00000a00  oly:24
    CP -12(r30), r8                                                  ; @00000a10  oly:25
    CP -16(r30), r9                                                  ; @00000a20  oly:25
    NOP                                                              ; @00000a30
    NOP                                                              ; @00000a40
    FROUND r8, r9, 0, 0, r10, r11                                    ; @00000a50  oly:25
    NOP                                                              ; @00000a60
    NOP                                                              ; @00000a70
    FROUND r10, r11, 0, 1, r8, r9                                    ; @00000a80  oly:25
    NOP                                                              ; @00000a90
    NOP                                                              ; @00000aa0
    FROUND r8, r9, 0, 2, r10, r11                                    ; @00000ab0  oly:25
    NOP                                                              ; @00000ac0
    NOP                                                              ; @00000ad0
    FROUND r10, r11, 0, 3, r8, r9                                    ; @00000ae0  oly:25
    NOP                                                              ; @00000af0
    NOP                                                              ; @00000b00
    AP -12(r30), r8                                                  ; @00000b10  oly:25
    SUMI r0, 8, r9 | AP -16(r30), r9                                 ; @00000b20  oly:26,25
    CP 0(r28), r8                                                    ; @00000b30  oly:26
    CP -20(r30), r10                                                 ; @00000b40  oly:26
    NOP                                                              ; @00000b50
    NOP                                                              ; @00000b60
    MUL r9, r10, r9                                                  ; @00000b70  oly:26
    NOP                                                              ; @00000b80
    NOP                                                              ; @00000b90
    SUM r8, r9, r8 | CP -12(r30), r9                                 ; @00000ba0  oly:26
    NOP                                                              ; @00000bb0
    NOP                                                              ; @00000bc0
    SUMI r0, 8, r9 | AP 0(r8), r9                                    ; @00000bd0  oly:27,26
    CP 0(r28), r8                                                    ; @00000be0  oly:27
    CP -20(r30), r10                                                 ; @00000bf0  oly:27
    NOP                                                              ; @00000c00
    NOP                                                              ; @00000c10
    MUL r9, r10, r9                                                  ; @00000c20  oly:27
    NOP                                                              ; @00000c30
    NOP                                                              ; @00000c40
    SUM r8, r9, r8 | CP -16(r30), r9                                 ; @00000c50  oly:27
    NOP                                                              ; @00000c60
    NOP                                                              ; @00000c70
    SUMI r8, 4, r8                                                   ; @00000c80  oly:27
    NOP                                                              ; @00000c90
    NOP                                                              ; @00000ca0
    AP 0(r8), r9                                                     ; @00000cb0  oly:27
    CP -20(r30), r8                                                  ; @00000cc0  oly:22
    NOP                                                              ; @00000cd0
    NOP                                                              ; @00000ce0
    SUMI r8, 1, r8                                                   ; @00000cf0  oly:22
    NOP                                                              ; @00000d00
    NOP                                                              ; @00000d10
    AP -20(r30), r8                                                  ; @00000d20  oly:22
L_0:
    CP -20(r30), r8                                                  ; @00000d30  oly:22
    CP 4(r28), r9                                                    ; @00000d40  oly:22
    NOP                                                              ; @00000d50
    NOP                                                              ; @00000d60
    SMQ r8, r9, L_1                                                  ; @00000d70  oly:22
    NOP                                                              ; @00000d80
    NOP                                                              ; @00000d90
    LOGOUT                                                           ; @00000da0  oly:29
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00000db0  oly:29
    CP -8(r30), r27                                                  ; @00000dc0  oly:29
    NOP                                                              ; @00000dd0
    NOP                                                              ; @00000de0
    SUM r27, r0, r30 | SRG r31                                       ; @00000df0  oly:29
