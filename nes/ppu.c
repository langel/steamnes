/*
	Picture Processing Unit
	ppu.c
*/

uint8_t ppu_addr[0x4000] = { 0 };
uint8_t ppu_oam[0x0100] = { 0 };

#define ppu_clock_div_ntsc  4
#define ppu_clock_div_pal   5
#define ppu_clock_div_dendy 5

int ppu_dot_frame_max = 89341;
int ppu_clock_div;
int ppu_write;
uint32_t ppu_dot_downcounter;

uint8_t  ppu_ctrl = 0;
uint8_t  ppu_mask = 0;
uint8_t  ppu_status = 0;
uint8_t  ppu_scroll_x = 0;
uint8_t  ppu_scroll_y = 0;
uint16_t ppu_scanline = 0;
uint16_t ppu_scan_dot = 0;
uint16_t ppu_pw;
uint16_t ppu_addr_bus;
uint8_t  ppu_addr_latch;
uint8_t  ppu_data_bus;

uint32_t * ppu_dot_data;
SDL_Texture * ppu_dot_texture;

void ppu_init() {
	ppu_dot_data = malloc(256 * 240 * 4);
	ppu_dot_texture = SDL_CreateTexture(fvc_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 256, 240);
}

void ppu_reset() {
	ppu_scanline = 0;
	ppu_scan_dot = 0;
	ppu_clock_div = ppu_clock_div_ntsc;
	ppu_write = 0;
}

void ppu_read_reg(int reg) {
	reg &= 0x7;
}

void ppu_write_reg(int reg) {
	reg &= 0x7;
}

void ppu_frame() {
	for (int i = 0x0000; i < 0x4000; i++) {
		ppu_dot_data[i] = nes_pal[ppu_addr[i] & 0x3f];
	}
	SDL_UpdateTexture(ppu_dot_texture, NULL, ppu_dot_data, 256);
	SDL_RenderCopy(fvc_renderer, ppu_dot_texture, NULL, &(SDL_Rect) { 47, 0, 256, 240 });
}

void ppu_dot() {
	if (ppu_scanline == 0 && ppu_scan_dot == 0) {
		// reset sprite 0 and sprite overflow bits
		ppu_status &= ~0x60;
	}
	if (ppu_scanline < 240) {
		// render space
	}
	else if (ppu_scanline == 240) {
		// post-render
	}
	else if (ppu_scanline == 241 && ppu_scan_dot == 1) {
		// set vblank bit flag
		ppu_status |= 0x80;
		nes_nmi++;
		ppu_frame_count++;
	}
	else if (ppu_scanline < 261) {
		// vertical blank
	}
	else {
		// pre-render
		if (ppu_scan_dot == 340 && (ppu_frame_count % 1)) {
			ppu_scanline = ppu_scan_dot = 0;
		}
	}
	ppu_scan_dot++;
	if (ppu_scan_dot > 340) {
		ppu_scan_dot = 0;
		ppu_scanline++;
		if (ppu_scanline > 261) ppu_scanline = 0;
	}
	ppu_cycle_count++;
}
