#!/data/data/com.termux/files/usr/bin/bash
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
COMPILER="$ROOT/tools/pawn-compiler-android/build-termux"
PAWNCC="$COMPILER/pawncc"

export LD_LIBRARY_PATH="$COMPILER"

"$PAWNCC" \
  -ptools/config/empty.inc \
  -itools/samp-stdlib \
  -itools/pawn-compiler-android/include \
  -ogamemodes/hopecity.amx \
  gamemodes/hopecity.pwn

echo ""
echo "[BUILD] HopeCity compiled successfully."
ls -lh gamemodes/hopecity.amx
