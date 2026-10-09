#!/bin/bash
# e2e_archivo.sh [texto|imagen]  Compila ejercicios/cifrar_archivo.oly, lo ejecuta en el RTL sobre un
# archivo real y compara el cifrado contra la referencia en C (feistel4_ref).
set -euo pipefail
VLIW_DIR="${VLIW_DIR:-../vliw_grupo2}"
JAF="${JAF:-./build/jaf}"
B="${E2E_OUT:-build/e2e_archivo}"
KEY=00112233445566778899aabbccddeeff
ADDR=0x20000
mkdir -p "$B"
make -s -C "$VLIW_DIR" build/tb_top.vvp build/feistel4_ref >/dev/null
python3 - <<PY
open("$B/original.bin","wb").write(("Compilador JAF -> VLIW Feistel4, CE1108 + CE4301. " * 6 + "fin\n").encode())
PY
SIZE=$(stat -c %s $B/original.bin); PAD=$(( (SIZE + 7) / 8 * 8 ))
"$JAF" -s --mem ejercicios/cifrar_archivo.oly -o $B/cifrar.bin
python3 $VLIW_DIR/tools/bin2mem.py $B/cifrar.bin $B/cifrar.mem
python3 $VLIW_DIR/tools/load_file.py --input $B/original.bin --output $B/datos.mem --address $ADDR >/dev/null
vvp -n $VLIW_DIR/build/tb_top.vvp +PROG=$B/cifrar.mem +DATA=$B/datos.mem +DUMP=$B/dump.txt \
    +DUMPLO=$((ADDR)) +DUMPHI=$((ADDR + PAD - 1)) +TIMEOUT=50000000 | grep -E "HALT|TIMEOUT"
python3 $VLIW_DIR/tools/extract_data.py --memory $B/dump.txt --address $ADDR --size $PAD --output $B/cifrado.bin
$VLIW_DIR/build/feistel4_ref enc $KEY $B/original.bin $B/cifrado_ref.bin
cmp $B/cifrado.bin $B/cifrado_ref.bin && echo "OK: el binario compilado desde JAF cifra igual que la referencia en C ($SIZE bytes)"
