CC      = gcc
CFLAGS  = -O2 -Wall -Wextra $(shell sdl2-config --cflags)
LIBS    = $(shell sdl2-config --libs) -lm
TARGET  = roblox_game
SRC     = src/main.c

# Linux/macOS
all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)
	@echo "Derleme tamam! ./$(TARGET) ile calistir"

# Windows cross-compile (mingw-w64)
windows:
	x86_64-w64-mingw32-gcc -O2 -Wall $(SRC) \
	  -I/usr/x86_64-w64-mingw32/include/SDL2 \
	  -L/usr/x86_64-w64-mingw32/lib \
	  -lSDL2 -lSDL2main -lm -mwindows \
	  -o $(TARGET).exe
	@echo "Windows EXE hazirlandi: $(TARGET).exe"

clean:
	rm -f $(TARGET) $(TARGET).exe
