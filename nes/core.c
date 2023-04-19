
#define clock_ntsc 21477272
#define clock_pal 26601712
#define clock_dendy clock_pal

int nes_running;

#include "./apu.c"
#include "./ppu.c"
#include "./cpu.c"

#include "./mbu.c"
