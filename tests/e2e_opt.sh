#!/bin/bash
# e2e_opt.sh   Prueba diferencial de la tuberia optimizadora en el RTL.
# Cada .oly con .expect se compila con cada configuracion de CONFIGS y se ejecuta
# en tb_top; la memoria final tiene que coincidir con el .expect (el mismo de P1).
# Uso: tests/e2e_opt.sh [fuentes...]   (por defecto ejercicios, e2e, enlace y opt)
VLIW_DIR="${VLIW_DIR:-../vliw_grupo2}"
export VLIW_DIR
CONFIGS=(
    "-O0"
    "-O1"
    "-O2"
    "--spill --max-regs=3"
    "-O2 --max-regs=4"
    "--unroll=3 --dce"
    "--rename --reorder --spill"
)
if [ $# -gt 0 ]; then
    FUENTES=("$@")
else
    FUENTES=(ejercicios/*.oly tests/e2e/*.oly tests/enlace/*.oly tests/opt/*.oly)
fi
pasan=0
fallan=0
for cfg in "${CONFIGS[@]}"; do
    etq=$(echo "$cfg" | tr -c 'A-Za-z0-9=\n' '_')
    for f in "${FUENTES[@]}"; do
        [ -f "$f" ] || continue
        [ -f "${f%.oly}.expect" ] || continue
        if JAFFLAGS="$cfg" E2E_OUT="build/e2e_opt/$etq" tests/e2e.sh "$f" > "build/e2e_opt_ultimo.log" 2>&1 &&
            grep -q "PASS" "build/e2e_opt_ultimo.log"; then
            pasan=$((pasan + 1))
        else
            fallan=$((fallan + 1))
            echo "FALLA [$cfg] $f"
            sed 's/^/    /' build/e2e_opt_ultimo.log | head -20
        fi
    done
done
echo "e2e_opt: $pasan pasaron, $fallan fallaron"
[ "$fallan" -eq 0 ]
