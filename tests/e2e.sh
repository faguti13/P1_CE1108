#!/bin/bash
# e2e.sh <fuente.oly> [expect]   Compila con jaf y ejecuta en el RTL (Icarus Verilog).
# Requiere VLIW_DIR (repo del procesador) y haber hecho `make -C $VLIW_DIR build/tb_top.vvp`.
set -e
SRC="$1"
VLIW_DIR="${VLIW_DIR:-../vliw_grupo2}"
EXPECT="${2:-${SRC%.oly}.expect}"
NAME="$(basename "${SRC%.oly}")"
OUT="${E2E_OUT:-build/e2e}"
JAF="${JAF:-./build/jaf}"
mkdir -p "$OUT"
"$JAF" -s --mem "$SRC" -o "$OUT/$NAME.bin"
python3 "$VLIW_DIR/tools/bin2mem.py" "$OUT/$NAME.bin" "$OUT/$NAME.mem"
vvp -n "$VLIW_DIR/build/tb_top.vvp" +PROG="$OUT/$NAME.mem" +REGS="$OUT/$NAME.regs" +DUMP="$OUT/$NAME.dump" \
    +DUMPLO=65536 +DUMPHI=${DUMPHI:-69631} +TIMEOUT=${TIMEOUT:-5000000} > "$OUT/$NAME.log"
python3 "$VLIW_DIR/tools/check_dump.py" --regs "$OUT/$NAME.regs" --dump "$OUT/$NAME.dump" \
    --expect "$EXPECT" --name "$NAME" --log "$OUT/$NAME.log"
grep -E "^HALT" "$OUT/$NAME.log" || true
