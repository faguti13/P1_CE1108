#!/bin/bash
# run_tests.sh - Pruebas del compilador sin simulador:
#   aceptados  deben compilar sin errores
#   errores    deben fallar con el codigo indicado en la linea "$ ESPERA: ..."
#   golden     el .asm generado coincide con tests/golden/*.asm
#   xcheck     jaf -x y vliwasm.py producen el mismo binario que el compilador
#   optimizado -O0/-O1/-O2 compilan, el .asm reensambla igual y --sim-ciclos termina
#   ejemplo    ejemplos/prueba.oly produce los 6 errores de tipo no soportado
JAF="${JAF:-./build/jaf}"
VLIW_DIR="${VLIW_DIR:-../vliw_grupo2}"
T=build/tests; mkdir -p $T
ok=0; fail=0
pass() { ok=$((ok+1)); echo "PASS $1"; }
bad()  { fail=$((fail+1)); echo "FAIL $1: $2"; }

for f in tests/aceptados/*.oly ejercicios/*.oly tests/e2e/*.oly tests/enlace/principal.oly; do
  n=$(basename $f .oly)
  if $JAF "$f" -o $T/$n.bin >$T/$n.out 2>&1; then pass "aceptado $n"; else bad "aceptado $n" "$(head -2 $T/$n.out)"; fi
done

for f in tests/errores/*.oly; do
  n=$(basename $f .oly)
  esp=$(sed -n '1s/^\$ ESPERA: //p' $f)
  $JAF "$f" -o $T/$n.bin >$T/$n.out 2>&1; rc=$?
  if [ $rc -ne 0 ] && grep -qF -- "$esp" $T/$n.out; then pass "error $n"; else bad "error $n" "rc=$rc esperaba '$esp' obtuvo: $(head -1 $T/$n.out)"; fi
done

for f in tests/golden/*.oly; do
  n=$(basename $f .oly)
  $JAF -s "$f" -o $T/$n.bin >/dev/null 2>&1
  if [ -n "$UPDATE_GOLDEN" ]; then cp $T/$n.asm tests/golden/$n.asm; fi
  if diff -q <(grep -v '^; generado' $T/$n.asm) <(grep -v '^; generado' tests/golden/$n.asm) >/dev/null; then pass "golden $n"; else bad "golden $n" "el .asm cambio (UPDATE_GOLDEN=1 para aceptar)"; fi
done

# jaf -x vs vliwasm.py vs el binario del propio compilador (dos ensambladores independientes)
for f in ejercicios/*.oly tests/e2e/*.oly tests/golden/*.oly; do
  n=$(basename $f .oly)
  $JAF -s "$f" -o $T/$n.a.bin >/dev/null 2>&1
  $JAF -x $T/$n.a.asm -o $T/$n.b.bin >/dev/null 2>&1
  if [ -f "$VLIW_DIR/tools/vliwasm.py" ]; then
    python3 $VLIW_DIR/tools/vliwasm.py $T/$n.a.asm --bin $T/$n.c.bin >/dev/null 2>&1
    if cmp -s $T/$n.a.bin $T/$n.b.bin && cmp -s $T/$n.a.bin $T/$n.c.bin; then pass "xcheck $n"; else bad "xcheck $n" "los binarios difieren"; fi
  else
    if cmp -s $T/$n.a.bin $T/$n.b.bin; then pass "xcheck $n (sin vliwasm.py)"; else bad "xcheck $n" "difiere"; fi
  fi
done

# los programas ensamblados a mano del repo del procesador tambien deben coincidir
if [ -d "$VLIW_DIR/programs" ]; then
  for f in $VLIW_DIR/programs/*.asm; do
    n=$(basename $f .asm)
    $JAF -x $f -o $T/hw_$n.a.bin >/dev/null 2>&1
    python3 $VLIW_DIR/tools/vliwasm.py $f --bin $T/hw_$n.b.bin >/dev/null 2>&1
    if cmp -s $T/hw_$n.a.bin $T/hw_$n.b.bin; then pass "xcheck programa $n"; else bad "xcheck programa $n" "difiere"; fi
  done
fi

# Tuberia optimizadora (P2): compila con -O0, -O1 y -O2; el .asm de -O2 se reensambla
# al mismo binario; con .expect, el simulador rapido llega a FIN sin riesgos RAW.
for f in tests/aceptados/*.oly ejercicios/*.oly tests/e2e/*.oly tests/opt/*.oly; do
  n=$(basename $f .oly); r=""
  for o in -O0 -O1 -O2; do
    $JAF $o -s "$f" -o $T/$n$o.bin >$T/$n$o.out 2>&1 || r="$o: $(head -1 $T/$n$o.out)"
  done
  if [ -z "$r" ]; then
    $JAF -x $T/$n-O2.asm -o $T/$n-O2.x.bin >/dev/null 2>&1 && cmp -s $T/$n-O2.bin $T/$n-O2.x.bin || r="el .asm de -O2 no reensambla igual"
  fi
  if [ -z "$r" ] && [ -f "${f%.oly}.expect" ]; then
    s=$($JAF -O2 --sim-ciclos "$f" -o $T/$n.sim.bin 2>&1)
    echo "$s" | grep -q "FIN alcanzado" && echo "$s" | grep -q "riesgos RAW sin resolver: 0," || r="sim: $(echo "$s" | head -1)"
  fi
  if [ -z "$r" ]; then pass "optimizado $n"; else bad "optimizado $n" "$r"; fi
done

# IDE: --ide-todo devuelve JSON con todas las vistas; cada error de tests/errores sale
# como diagnostico con su codigo (los lexicos con codigo 1) y linea > 0.
for f in ejercicios/ej3_feistel.oly tests/opt/spill_anidado_llamadas.oly tests/errores/*.oly; do
  n=$(basename $f .oly)
  esp=$(sed -n '1s/^\$ ESPERA: {\{0,1\}ERROR 0*\([0-9]*\)}.*/\1/p' $f)
  case $f in tests/errores/l*) esp=1 ;; esac
  $JAF --ide-todo "$f" -O2 >$T/$n.ide.json 2>/dev/null
  r=$(python3 - "$T/$n.ide.json" "$esp" <<'EOF'
import json, sys
d = json.load(open(sys.argv[1]))
faltan = {"diagnostics", "fixes", "completions", "clones", "simbolos", "ast", "ir", "paquetes", "metricas"} - set(d)
if faltan:
    print("faltan", sorted(faltan))
elif sys.argv[2]:
    if not any(int(x["code"]) == int(sys.argv[2]) and x["line"] > 0 for x in d["diagnostics"]):
        print("sin diagnostico", sys.argv[2], d["diagnostics"][:1])
elif d["diagnostics"] or not d["metricas"]["opt"]["ok"] or not d["paquetes"]["opt"] or not d["simbolos"]:
    print("vistas vacias o errores inesperados")
EOF
)
  if [ -z "$r" ]; then pass "ide $n"; else bad "ide $n" "$r"; fi
done

n=$($JAF ejemplos/prueba.oly -o $T/prueba.bin 2>&1 | grep -c "ERROR 14")
if [ "$n" = 6 ]; then pass "ejemplos/prueba.oly rechaza real/doble/caract/cad (6 errores de tipo no soportado)"; else bad "prueba.oly" "ERROR 14 x$n"; fi
$JAF --arbol ejemplos/prueba.oly >/dev/null 2>&1 && pass "ejemplos/prueba.oly parsea" || bad "prueba.oly" "no parsea"

echo "== $ok pasaron, $fail fallaron =="
[ $fail -eq 0 ]
