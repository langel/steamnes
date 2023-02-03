/*
	Picture Processing Unit
	ppu.c
*/

const int ppu_clock_div_ntsc = 4;
const int ppu_clock_div_pal = 5;
const int ppu_clock_div_dendy = 5;

int ppu_dot_count;
int ppu_clock_div;

uint8_t ppu_regs[8];
uint16_t ppu_addr_bus;
uint8_t ppu_data_bus;

void ppu_reset() {
	ppu_dot_count = 0;
	ppu_clock_div = ppu_clock_div_ntsc;
}

void ppu_dot() {
}
