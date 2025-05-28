#!/bin/bash

if [ -z "$1" ]; then
    echo "Uso: $0 <id> <ninja_numbers>"
    echo "  ninja_numbers: uno o más números separados por espacios (1 2 3), o ALL para todos"
    exit 1
fi

id="$1"
shift

if [ $# -eq 0 ]; then
    echo "Error: Debes especificar al menos un ninja o ALL"
    exit 1
fi

declare -a ninjas=()

if [[ "$@" == "ALL" ]]; then
    ninjas=(1 2 3)
else
    for arg in "$@"; do
        if [[ "$arg" =~ ^[123]$ ]]; then
            ninjas+=("$arg")
        else
            echo "Advertencia: Ninja inválido '$arg' ignorado. Solo se permiten 1, 2, 3 o ALL."
        fi
    done
fi

if [ ${#ninjas[@]} -eq 0 ]; then
    echo "No se especificaron ninjas válidos. Abortando."
    exit 1
fi

for ninja in "${ninjas[@]}"; do
  for pos in 1 2; do
    (
      echo "Ejecutando Ninja $ninja contra jugador $pos..."
      logfile="log_temp/log_ninja${ninja}_pos${pos}.txt"

      if [ "$pos" -eq 1 ]; then
        ./ParchisGame --p1 AI "$id" "$id" --p2 NINJA "$ninja" N --no-gui > "$logfile" 2>&1
      else
        ./ParchisGame --p2 AI "$id" "$id" --p1 NINJA "$ninja" N --no-gui > "$logfile" 2>&1
      fi

      echo "Resultado ejecución para Ninja $ninja contra jugador $pos:"
      tail -n 5 "$logfile"
      echo "Fin de ejecución Ninja $ninja contra jugador $pos"
    ) &
    sleep 0.3  # ligera pausa para no saturar
  done
done

wait
echo "Todas las ejecuciones han finalizado."
rm -rf log_temp/*

