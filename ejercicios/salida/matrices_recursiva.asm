; generado por jaf desde ejercicios/matrices_recursiva.oly
; un bundle por linea, slots 0..3 separados por '|'
; @xxxxxxxx es el byte del bundle en el .bin (cada bundle ocupa 16)
; oly:N es la linea del fuente que produjo la instruccion
_inicio:
    SUMI r0, 32, r28                                                 ; @00000000  oly:1
    SUMI r0, 512, r29                                                ; @00000010  oly:1
    SUMI r0, 2, r8                                                   ; @00000020  oly:7
    DLII r28, 11, r28                                                ; @00000030  oly:1
    DLII r29, 11, r29                                                ; @00000040  oly:1
    SUMI r0, 1, r10                                                  ; @00000050  oly:7
    SUMI r28, 0, r9                                                  ; @00000060  oly:7
    SUM r29, r0, r30                                                 ; @00000070  oly:1
    SUMI r29, -64, r29                                               ; @00000080  oly:1
    SUMI r0, 3, r8 | AP 0(r9), r8                                    ; @00000090  oly:7
    NOP                                                              ; @000000a0
    NOP                                                              ; @000000b0
    SUMI r0, 0, r8 | AP 4(r9), r8                                    ; @000000c0  oly:8,7
    SUMI r0, 2, r10 | AP 8(r9), r10                                  ; @000000d0  oly:7
    NOP                                                              ; @000000e0
    NOP                                                              ; @000000f0
    SUMI r0, 3, r10 | AP 12(r9), r10                                 ; @00000100  oly:7
    NOP                                                              ; @00000110
    NOP                                                              ; @00000120
    SUMI r0, 4, r10 | AP 16(r9), r10                                 ; @00000130  oly:7
    NOP                                                              ; @00000140
    NOP                                                              ; @00000150
    SUMI r0, 5, r10 | AP 20(r9), r10                                 ; @00000160  oly:7
    NOP                                                              ; @00000170
    NOP                                                              ; @00000180
    SUMI r0, 6, r10 | AP 24(r9), r10                                 ; @00000190  oly:7
    NOP                                                              ; @000001a0
    NOP                                                              ; @000001b0
    AP 28(r9), r10                                                   ; @000001c0  oly:7
    SUMI r0, 0, r8 | AP 32(r28), r8                                  ; @000001d0  oly:9,8
    NOP                                                              ; @000001e0
    NOP                                                              ; @000001f0
    SUMI r0, 0, r8 | AP 36(r28), r8                                  ; @00000200  oly:10,9
    NOP                                                              ; @00000210
    NOP                                                              ; @00000220
    AP 40(r28), r8 | SYE f_main                                      ; @00000230  oly:10
    FIN                                                              ; @00000240  oly:10
f_suma_fila:
    SUM r29, r0, r30 | AP -24(r29), r30                              ; @00000250  oly:12
    SUMI r29, -88, r29 | AP -20(r29), r31                            ; @00000260  oly:12
    NOP                                                              ; @00000270
    CP -12(r30), r8                                                  ; @00000280  oly:13
    CP -16(r30), r9                                                  ; @00000290  oly:13
    NOP                                                              ; @000002a0
    NOP                                                              ; @000002b0
    SMQ r8, r9, L_0                                                  ; @000002c0  oly:13
    NOP                                                              ; @000002d0
    NOP                                                              ; @000002e0
    SUMI r0, 0, r8                                                   ; @000002f0  oly:14
    NOP                                                              ; @00000300
    NOP                                                              ; @00000310
    SUM r8, r0, r1 | S f_suma_fila_fin                               ; @00000320  oly:14
L_0:
    CP -4(r30), r8                                                   ; @00000330  oly:16
    CP -8(r30), r9                                                   ; @00000340  oly:16
    CP -12(r30), r10                                                 ; @00000350  oly:16
    CP 4(r8), r11                                                    ; @00000360  oly:16
    NOP                                                              ; @00000370
    NOP                                                              ; @00000380
    MUL r9, r11, r9                                                  ; @00000390  oly:16
    NOP                                                              ; @000003a0
    NOP                                                              ; @000003b0
    SUM r9, r10, r9                                                  ; @000003c0  oly:16
    NOP                                                              ; @000003d0
    NOP                                                              ; @000003e0
    DLII r9, 2, r9                                                   ; @000003f0  oly:16
    NOP                                                              ; @00000400
    NOP                                                              ; @00000410
    SUM r8, r9, r8                                                   ; @00000420  oly:16
    NOP                                                              ; @00000430
    NOP                                                              ; @00000440
    CP 8(r8), r8                                                     ; @00000450  oly:16
    NOP                                                              ; @00000460
    NOP                                                              ; @00000470
    AP -28(r30), r8                                                  ; @00000480  oly:16
    CP -4(r30), r9                                                   ; @00000490  oly:16
    CP -8(r30), r10                                                  ; @000004a0  oly:16
    CP -12(r30), r11                                                 ; @000004b0  oly:16
    CP -16(r30), r12                                                 ; @000004c0  oly:16
    AP -4(r29), r9                                                   ; @000004d0  oly:16
    SUMI r11, 1, r11 | AP -8(r29), r10                               ; @000004e0  oly:16
    NOP                                                              ; @000004f0
    NOP                                                              ; @00000500
    AP -12(r29), r11                                                 ; @00000510  oly:16
    AP -16(r29), r12 | SYE f_suma_fila                               ; @00000520  oly:16
    SUM r1, r0, r9 | CP -28(r30), r8                                 ; @00000530  oly:16
    NOP                                                              ; @00000540
    NOP                                                              ; @00000550
    SUM r8, r9, r8                                                   ; @00000560  oly:16
    NOP                                                              ; @00000570
    NOP                                                              ; @00000580
    SUM r8, r0, r1                                                   ; @00000590  oly:16
    NOP                                                              ; @000005a0
    NOP                                                              ; @000005b0
f_suma_fila_fin:
    SUM r30, r0, r29 | CP -20(r30), r31                              ; @000005c0  oly:16
    CP -24(r30), r27                                                 ; @000005d0  oly:16
    NOP                                                              ; @000005e0
    NOP                                                              ; @000005f0
    SUM r27, r0, r30 | SRG r31                                       ; @00000600  oly:16
f_suma_matriz:
    SUM r29, r0, r30 | AP -24(r29), r30                              ; @00000610  oly:19
    SUMI r29, -88, r29 | AP -20(r29), r31                            ; @00000620  oly:19
    NOP                                                              ; @00000630
    CP -8(r30), r8                                                   ; @00000640  oly:20
    CP -12(r30), r9                                                  ; @00000650  oly:20
    NOP                                                              ; @00000660
    NOP                                                              ; @00000670
    SMQ r8, r9, L_1                                                  ; @00000680  oly:20
    NOP                                                              ; @00000690
    NOP                                                              ; @000006a0
    SUMI r0, 0, r8                                                   ; @000006b0  oly:21
    NOP                                                              ; @000006c0
    NOP                                                              ; @000006d0
    SUM r8, r0, r1 | S f_suma_matriz_fin                             ; @000006e0  oly:21
L_1:
    SUMI r0, 0, r10 | CP -4(r30), r8                                 ; @000006f0  oly:23
    CP -8(r30), r9                                                   ; @00000700  oly:23
    CP -16(r30), r11                                                 ; @00000710  oly:23
    AP -4(r29), r8                                                   ; @00000720  oly:23
    AP -8(r29), r9                                                   ; @00000730  oly:23
    AP -12(r29), r10                                                 ; @00000740  oly:23
    AP -16(r29), r11 | SYE f_suma_fila                               ; @00000750  oly:23
    SUM r1, r0, r8                                                   ; @00000760  oly:23
    NOP                                                              ; @00000770
    NOP                                                              ; @00000780
    AP -28(r30), r8                                                  ; @00000790  oly:23
    CP -4(r30), r9                                                   ; @000007a0  oly:23
    CP -8(r30), r10                                                  ; @000007b0  oly:23
    CP -12(r30), r11                                                 ; @000007c0  oly:23
    CP -16(r30), r12                                                 ; @000007d0  oly:23
    SUMI r10, 1, r10 | AP -4(r29), r9                                ; @000007e0  oly:23
    NOP                                                              ; @000007f0
    NOP                                                              ; @00000800
    AP -8(r29), r10                                                  ; @00000810  oly:23
    AP -12(r29), r11                                                 ; @00000820  oly:23
    AP -16(r29), r12 | SYE f_suma_matriz                             ; @00000830  oly:23
    SUM r1, r0, r9 | CP -28(r30), r8                                 ; @00000840  oly:23
    NOP                                                              ; @00000850
    NOP                                                              ; @00000860
    SUM r8, r9, r8                                                   ; @00000870  oly:23
    NOP                                                              ; @00000880
    NOP                                                              ; @00000890
    SUM r8, r0, r1                                                   ; @000008a0  oly:23
    NOP                                                              ; @000008b0
    NOP                                                              ; @000008c0
f_suma_matriz_fin:
    SUM r30, r0, r29 | CP -20(r30), r31                              ; @000008d0  oly:23
    CP -24(r30), r27                                                 ; @000008e0  oly:23
    NOP                                                              ; @000008f0
    NOP                                                              ; @00000900
    SUM r27, r0, r30 | SRG r31                                       ; @00000910  oly:23
f_fib:
    SUM r29, r0, r30 | AP -12(r29), r30                              ; @00000920  oly:26
    SUMI r29, -76, r29 | AP -8(r29), r31                             ; @00000930  oly:26
    SUMI r0, 2, r9                                                   ; @00000940  oly:27
    CP -4(r30), r8                                                   ; @00000950  oly:27
    NOP                                                              ; @00000960
    NOP                                                              ; @00000970
    SMI r8, r9, L_2                                                  ; @00000980  oly:27
    NOP                                                              ; @00000990
    NOP                                                              ; @000009a0
    CP -4(r30), r8                                                   ; @000009b0  oly:28
    NOP                                                              ; @000009c0
    NOP                                                              ; @000009d0
    SUM r8, r0, r1 | S f_fib_fin                                     ; @000009e0  oly:28
L_2:
    CP -4(r30), r8                                                   ; @000009f0  oly:30
    NOP                                                              ; @00000a00
    NOP                                                              ; @00000a10
    RESTI r8, 1, r8                                                  ; @00000a20  oly:30
    NOP                                                              ; @00000a30
    NOP                                                              ; @00000a40
    AP -4(r29), r8 | SYE f_fib                                       ; @00000a50  oly:30
    SUM r1, r0, r8                                                   ; @00000a60  oly:30
    NOP                                                              ; @00000a70
    NOP                                                              ; @00000a80
    AP -16(r30), r8                                                  ; @00000a90  oly:30
    CP -4(r30), r9                                                   ; @00000aa0  oly:30
    NOP                                                              ; @00000ab0
    NOP                                                              ; @00000ac0
    RESTI r9, 2, r9                                                  ; @00000ad0  oly:30
    NOP                                                              ; @00000ae0
    NOP                                                              ; @00000af0
    AP -4(r29), r9 | SYE f_fib                                       ; @00000b00  oly:30
    SUM r1, r0, r9 | CP -16(r30), r8                                 ; @00000b10  oly:30
    NOP                                                              ; @00000b20
    NOP                                                              ; @00000b30
    SUM r8, r9, r8                                                   ; @00000b40  oly:30
    NOP                                                              ; @00000b50
    NOP                                                              ; @00000b60
    SUM r8, r0, r1                                                   ; @00000b70  oly:30
    NOP                                                              ; @00000b80
    NOP                                                              ; @00000b90
f_fib_fin:
    SUM r30, r0, r29 | CP -8(r30), r31                               ; @00000ba0  oly:30
    CP -12(r30), r27                                                 ; @00000bb0  oly:30
    NOP                                                              ; @00000bc0
    NOP                                                              ; @00000bd0
    SUM r27, r0, r30 | SRG r31                                       ; @00000be0  oly:30
f_fact:
    SUM r29, r0, r30 | AP -12(r29), r30                              ; @00000bf0  oly:33
    SUMI r29, -76, r29 | AP -8(r29), r31                             ; @00000c00  oly:33
    SUMI r0, 1, r9                                                   ; @00000c10  oly:34
    CP -4(r30), r8                                                   ; @00000c20  oly:34
    NOP                                                              ; @00000c30
    NOP                                                              ; @00000c40
    SMQ r9, r8, L_3                                                  ; @00000c50  oly:34
    NOP                                                              ; @00000c60
    NOP                                                              ; @00000c70
    SUMI r0, 1, r8                                                   ; @00000c80  oly:35
    NOP                                                              ; @00000c90
    NOP                                                              ; @00000ca0
    SUM r8, r0, r1 | S f_fact_fin                                    ; @00000cb0  oly:35
L_3:
    CP -4(r30), r8                                                   ; @00000cc0  oly:37
    NOP                                                              ; @00000cd0
    NOP                                                              ; @00000ce0
    AP -16(r30), r8                                                  ; @00000cf0  oly:37
    CP -4(r30), r9                                                   ; @00000d00  oly:37
    NOP                                                              ; @00000d10
    NOP                                                              ; @00000d20
    RESTI r9, 1, r9                                                  ; @00000d30  oly:37
    NOP                                                              ; @00000d40
    NOP                                                              ; @00000d50
    AP -4(r29), r9 | SYE f_fact                                      ; @00000d60  oly:37
    SUM r1, r0, r9 | CP -16(r30), r8                                 ; @00000d70  oly:37
    NOP                                                              ; @00000d80
    NOP                                                              ; @00000d90
    MUL r8, r9, r8                                                   ; @00000da0  oly:37
    NOP                                                              ; @00000db0
    NOP                                                              ; @00000dc0
    SUM r8, r0, r1                                                   ; @00000dd0  oly:37
    NOP                                                              ; @00000de0
    NOP                                                              ; @00000df0
f_fact_fin:
    SUM r30, r0, r29 | CP -8(r30), r31                               ; @00000e00  oly:37
    CP -12(r30), r27                                                 ; @00000e10  oly:37
    NOP                                                              ; @00000e20
    NOP                                                              ; @00000e30
    SUM r27, r0, r30 | SRG r31                                       ; @00000e40  oly:37
f_main:
    SUM r29, r0, r30 | AP -8(r29), r30                               ; @00000e50  oly:40
    SUMI r29, -72, r29 | AP -4(r29), r31                             ; @00000e60  oly:40
    SUMI r28, 0, r8                                                  ; @00000e70  oly:41
    SUMI r0, 0, r9                                                   ; @00000e80  oly:41
    SUMI r0, 2, r10                                                  ; @00000e90  oly:41
    SUMI r0, 3, r11 | AP -4(r29), r8                                 ; @00000ea0  oly:41
    AP -8(r29), r9                                                   ; @00000eb0  oly:41
    AP -12(r29), r10                                                 ; @00000ec0  oly:41
    AP -16(r29), r11 | SYE f_suma_matriz                             ; @00000ed0  oly:41
    SUM r1, r0, r8                                                   ; @00000ee0  oly:41
    NOP                                                              ; @00000ef0
    NOP                                                              ; @00000f00
    SUMI r0, 10, r8 | AP 32(r28), r8                                 ; @00000f10  oly:42,41
    NOP                                                              ; @00000f20
    NOP                                                              ; @00000f30
    AP -4(r29), r8 | SYE f_fib                                       ; @00000f40  oly:42
    SUM r1, r0, r8                                                   ; @00000f50  oly:42
    NOP                                                              ; @00000f60
    NOP                                                              ; @00000f70
    SUMI r0, 6, r8 | AP 36(r28), r8                                  ; @00000f80  oly:43,42
    NOP                                                              ; @00000f90
    NOP                                                              ; @00000fa0
    AP -4(r29), r8 | SYE f_fact                                      ; @00000fb0  oly:43
    SUM r1, r0, r8                                                   ; @00000fc0  oly:43
    NOP                                                              ; @00000fd0
    NOP                                                              ; @00000fe0
    AP 40(r28), r8                                                   ; @00000ff0  oly:43
f_main_fin:
    SUM r30, r0, r29 | CP -4(r30), r31                               ; @00001000  oly:43
    CP -8(r30), r27                                                  ; @00001010  oly:43
    NOP                                                              ; @00001020
    NOP                                                              ; @00001030
    SUM r27, r0, r30 | SRG r31                                       ; @00001040  oly:43
