#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "./fcl/core.c"
#include "./fcl/util/debug.c"
#include "./nes/core.c"
#include "./emu/core.c"



int main(int argc, char* args[]) {
	debug_output_target = all;
	debug_level = 4;
	debug_out(1, "SteamNES");
	int error = rom_load("game_rom.nes");
	if (error) debug_out(0, "failed to load or identify rom file");
	mbu_start();
	return 0;
}
