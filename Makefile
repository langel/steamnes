CC ?= gcc
TARGET ?= main.exe
SRC ?= main.c

CFLAGS += -g -Wall -std=c99

# Prefer pkg-config when available (MSYS2, Linux, etc.)
SDL2_CFLAGS := $(shell pkg-config --cflags sdl2 2>NUL)
SDL2_LIBS := $(shell pkg-config --libs sdl2 2>NUL)

# Fallback autodetect for common Windows installs.
SDL2_PREFIX :=
ifneq ($(wildcard C:/Users/$(USERNAME)/source/msys/mingw64/include/SDL2/SDL.h),)
	SDL2_PREFIX := C:/Users/$(USERNAME)/source/msys/mingw64
else ifneq ($(wildcard C:/Users/$(USERNAME)/source/w64devkit/include/SDL2/SDL.h),)
	SDL2_PREFIX := C:/Users/$(USERNAME)/source/w64devkit
else ifneq ($(wildcard C:/w64devkit/include/SDL2/SDL.h),)
	SDL2_PREFIX := C:/w64devkit
endif

ifeq ($(strip $(SDL2_CFLAGS)),)
ifeq ($(strip $(SDL2_PREFIX)),)
$(error Could not find SDL2 headers. Install SDL2 or set SDL2_CFLAGS and SDL2_LIBS.)
endif
	SDL2_CFLAGS := -I$(SDL2_PREFIX)/include/SDL2 -Dmain=SDL_main
	SDL2_LIBS := -L$(SDL2_PREFIX)/lib -lmingw32 -lSDL2main -lSDL2 -mwindows -mconsole
endif

.PHONY: build clean

build:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(SDL2_CFLAGS) $(SDL2_LIBS)

clean:
	rm -f $(TARGET)
