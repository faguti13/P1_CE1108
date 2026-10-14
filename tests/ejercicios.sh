#!/bin/bash
# ejercicios.sh - Compila cada ejercicio, lo ejecuta en el RTL y versiona la salida en ejercicios/salida/:
#   <n>.asm, <n>.bin, <n>.paquetes.txt (direccion, hexadecimal y ensamblador), <n>.traza.txt (primeros ciclos
#   del simulador), <n>.esperado (reglas de check_dump) y <n>.resultado.txt (HALT y PASS/FAIL).
set -e
VLIW_DIR="${VLIW_DIR:-../vliw_grupo2}"
JAF="${JAF:-./build/jaf}"
S=ejercicios/salida; mkdir -p $S build/ej
for f in ejercicios/*.oly; do
  n=$(basename $f .oly)
  $JAF -s -p --mem "$f" -o $S/$n.bin > $S/$n.paquetes.txt
  python3 $VLIW_DIR/tools/bin2mem.py $S/$n.bin build/ej/$n.mem
  rm -f $S/$n.mem
  if [ -f ejercicios/$n.expect ]; then
    cp ejercicios/$n.expect $S/$n.esperado
    vvp -n $VLIW_DIR/build/tb_top.vvp +PROG=build/ej/$n.mem +TRACE +TIMEOUT=5000000 +REGS=build/ej/$n.regs \
        +DUMP=build/ej/$n.dump +DUMPLO=65536 +DUMPHI=69631 > build/ej/$n.log
    head -300 build/ej/$n.log > $S/$n.traza.txt
    { grep -E "^HALT" build/ej/$n.log
      python3 $VLIW_DIR/tools/check_dump.py --regs build/ej/$n.regs --dump build/ej/$n.dump \
        --expect ejercicios/$n.expect --name $n --log build/ej/$n.log; } > $S/$n.resultado.txt
    cat $S/$n.resultado.txt
  fi
done
