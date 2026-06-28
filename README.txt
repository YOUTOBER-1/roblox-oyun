===============================================
   ROBLOX TARZLI OYUN - C ile Yapildi!
   SDL2 + OpenGL Raycasting 3D Engine
===============================================

OYUN HAKKINDA:
  - 3D Raycasting motoru (Wolfenstein/DOOM tarzi)
  - 30 altin coin topla
  - 5 dusmandan kac veya onlari vur
  - Minimap ile haritayi gor
  - 100 HP baslarsın, dusmanlar hasar verir

KONTROLLER:
  W / YUK ARK  - Ileri git
  S / ASAGI    - Geri git
  A            - Sola kaydir
  D            - Saga kaydir
  Q / SOL OK   - Sola don
  E / SAG OK   - Saga don
  FARE         - Kamera (sol tik ile aktif et)
  SOL TIK / F  - Ates et (dusmanı vur)
  SPACE/ENTER  - Baslat / Yeniden baslat
  ESC          - Menuye don / Cikis

HEDEF:
  Tum coinleri topla = KAZANDIN!
  HP 0'a duserse   = KAYBETTIN!

SKOR:
  Coin toplama: +10 puan
  Dusman vurma: +50 puan

===============================================
WINDOWS ICIN DERLEME:
===============================================

Gereksinimler:
  - MinGW-w64 (https://www.mingw-w64.org/)
  - SDL2 Windows gelistirici paketi
    (https://github.com/libsdl-org/SDL/releases)

Derleme komutu (MinGW):
  gcc -O2 src/main.c -I<SDL2_INCLUDE> -L<SDL2_LIB> -lSDL2 -lSDL2main -lm -mwindows -o roblox_game.exe

SDL2.dll'i .exe ile ayni klasore koy!

===============================================
LINUX ICIN DERLEME:
===============================================

  sudo apt install libsdl2-dev   # Ubuntu/Debian
  bash build.sh                  # Derle
  ./roblox_game                  # Oyna!

===============================================
