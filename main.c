#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./nes/main.c"
#include "./emu/main.c"



int main(int argc, char* args[]) {
	printf("SteamNES\n");
	rom_load("game_rom.nes");
	return 0;
}
