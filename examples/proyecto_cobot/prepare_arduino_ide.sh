#!/usr/bin/env bash
# Genera un sketch plano para Arduino IDE e instala la librería HerkuleX.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
LIB_ROOT="$(cd "$ROOT/../.." && pwd)"
SKETCH="$ROOT/arduino_ide/sketch"
LIB_DEST="${ARDUINO_LIB_DIR:-$HOME/Arduino/libraries/HerkulexDRS0602}"

echo "==> Instalando librería en $LIB_DEST"
mkdir -p "$LIB_DEST"
cp -f "$LIB_ROOT/src/"* "$LIB_DEST/"

echo "==> Generando sketch en $SKETCH"
rm -rf "$SKETCH"
mkdir -p "$SKETCH"
cp -f "$ROOT/src/"*.cpp "$ROOT/src/"*.h "$SKETCH/"
cp -f "$ROOT/config/"*.h "$SKETCH/"
mv "$SKETCH/main.cpp" "$SKETCH/proyecto_cobot.ino"

echo
echo "Listo. Pasos en Arduino IDE:"
echo "  1. Archivo → Abrir → $SKETCH/proyecto_cobot.ino"
echo "  2. Herramientas → Placa → Arduino Mega or Mega 2560"
echo "  3. Herramientas → Puerto → (tu Mega por USB)"
echo "  4. Subir y abrir Monitor Serie a 115200 baud"
echo
echo "Nota: volvé a ejecutar este script si cambiás archivos en src/ o config/."
