/*
	Picture Processing Unit
	ppu.c
*/

uint8_t ppu_ram[0x4000] = { 0 };
uint8_t ppu_oam[0x0100] = { 0 };

#define ppu_clock_div_ntsc  4
#define ppu_clock_div_pal   5
#define ppu_clock_div_dendy 5

int ppu_dot_count;
int ppu_frames;
int ppu_dot_frame_max;
int ppu_clock_div;
int ppu_write;
uint32_t ppu_dot_downcounter;

uint8_t ppu_ctrl = 0;
uint8_t ppu_mask = 0;
uint8_t ppu_status = 0;
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
	ppu_frames = 0;
	ppu_dot_frame_max = 89341;
	ppu_clock_div = ppu_clock_div_ntsc;
	ppu_dot_downcounter = ppu_dot_frame_max;
	ppu_write = 0;
	for (int i = 0; i < 8; i++) {
		ppu_regs[i] = 0;
	}
}

void ppu_read_reg(int reg) {
	reg &= 0x7;
}

void ppu_write_reg(int reg) {
	reg &= 0x7;
}

void ppu_dot() {
	if (ppu_int == 1) ppu_int = 0;
	if (!ppu_dot_downcounter) {
		ppu_int = 1;
		ppu_dot_downcounter = ppu_dot_frame_max;
		//ppu_dot_count = 0;
		ppu_frames++;
		nes_nmi++;
		//debug_out(3, "ppu trigger nmi %dx", nes_nmi);
	}
	ppu_dot_downcounter--;
	ppu_dot_count++;
	ppu_cycle_count++;
}
