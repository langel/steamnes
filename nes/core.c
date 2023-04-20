
#define clock_ntsc 21477272
#define clock_pal 26601712
#define clock_dendy clock_pal

int nes_running;
uint32_t cpu_cycle_count;
uint32_t ppu_cycle_count;
uint32_t mbu_cycle_count;

#include "./apu.c"
#include "./ppu.c"
#include "./cpu.c"

#include "./mbu.c"
