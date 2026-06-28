#!/bin/bash
set -e

echo "=== Roblox Tarzi Oyun - Derleme Scripti ==="
echo ""

# SDL2 kontrol
if ! command -v sdl2-config &>/dev/null; then
    # Nix store'dan bul
    SDL2_CONFIG=$(find /nix/store -name "sdl2-config" 2>/dev/null | head -1)
    if [ -z "$SDL2_CONFIG" ]; then
        echo "HATA: SDL2 bulunamadi!"
        exit 1
    fi
    CFLAGS=$($SDL2_CONFIG --cflags)
    LIBS=$($SDL2_CONFIG --libs)
else
    CFLAGS=$(sdl2-config --cflags)
    LIBS=$(sdl2-config --libs)
fi

echo "SDL2 bulundu."
echo "Derleniyor..."

gcc -O2 -Wall src/main.c \
    $CFLAGS \
    $LIBS \
    -lm \
    -o roblox_game

echo ""
echo "=== BASARILI! ==="
echo "Oynamak icin: ./roblox_game"
echo ""
echo "KONTROLLER:"
echo "  WASD       - Hareket"
echo "  Q/E        - Sol/Sag don"
echo "  FARE       - Kamera (tiklayinca aktif olur)"
echo "  SOL TIK / F - Ates et"
echo "  ESC        - Menuye don"
echo "  ENTER/BOSLUK - Baslat"
