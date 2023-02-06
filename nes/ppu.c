/*
	Picture Processing Unit
	ppu.c
*/

const int ppu_clock_div_ntsc = 4;
const int ppu_clock_div_pal = 5;
const int ppu_clock_div_dendy = 5;

int ppu_dot_count;
int ppu_dot_frame_max;
int ppu_clock_div;

uint8_t ppu_regs[8];
uint16_t ppu_addr_bus;
uint8_t ppu_data_bus;

// pins
uint8_t ppu_int; // NMI
uint8_t ppu_rw_in;
uint8_t ppu_rd_out;
uint8_t ppu_wr_out;

void ppu_reset() {
	ppu_dot_count = 0;
	ppu_dot_frame_max = 89341;
	ppu_clock_div = ppu_clock_div_ntsc;
	for (int i = 0; i < 8; i++) {
		ppu_regs[i] = 0;
	}
}

void ppu_dot() {
	if (ppu_int == 1) ppu_int = 0;
	if (ppu_dot_count >= ppu_dot_frame_max) {
		ppu_int = 1;
		ppu_dot_count = 0;
	}
	ppu_dot_count++;
}
