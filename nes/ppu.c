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
	ppu_dot_texture = SDL_CreateTexture(fvc_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	dpipe_init("nes/ppu.c");
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

void ppu_sprite_evaluation(int scanline) {
}

void ppu_frame() {
	for (int i = 0x0000; i < 0x4000; i++) {
//		ppu_dot_data[i] = nes_pal[ppu_addr[i] & 0x3f];
	}
	SDL_UpdateTexture(ppu_dot_texture, NULL, ppu_dot_data, 256 * 4);
	SDL_RenderCopy(fvc_renderer, ppu_dot_texture, NULL, &(SDL_Rect) { 47, 0, 256, 240 });
	dpipe_update();
}

void ppu_dot() {
	static int pxl_pos;
	if (ppu_scanline == 0 && ppu_scan_dot == 0) {
		// reset sprite 0 and sprite overflow bits
		ppu_status &= ~0x60;
		pxl_pos = 0;
	}
	if (ppu_scanline < 240) {
		// render space
		if (ppu_scan_dot < 256) {
			// name table
			int nt = 0x2000 + ((ppu_ctrl & 0x03) << 10);
			// pattern table
			int pt = (ppu_ctrl & 0x10) ? 0x1000 : 0x0000;
			// find dot pattern
			int tile_id = ppu_addr[nt + (ppu_scan_dot >> 3) + ((ppu_scanline >> 3) << 5)];
			int tile_x = ppu_scan_dot & 0x07;
			int tile_col = 1 << (0x07 - tile_x);
			int tile_y = ppu_scanline & 0x07;
			int tile_offset = pt + (tile_id << 4) + tile_y;
			int tile_row_lo = ppu_addr[tile_offset];
			int tile_row_hi = ppu_addr[tile_offset + 8];
			int tile_pal = 0;
			int color = (tile_row_lo & tile_col) ? 1 : 0;
			color += (tile_row_hi & tile_col) ? 2 : 0;
			ppu_dot_data[pxl_pos] = nes_pal[ppu_addr[0x3f00 + (tile_pal << 2) + color]];
			pxl_pos++;
		}
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
		/*
		if (ppu_scan_dot == 340 && (ppu_frame_count % 1)) {
			ppu_scan_dot = 0;
			ppu_scanline = 0;
		}
		*/
	}
	ppu_scan_dot++;
	if (ppu_scan_dot > 340) {
		ppu_scan_dot = 0;
		ppu_scanline++;
		if (ppu_scanline > 261) ppu_scanline = 0;
	}
	ppu_cycle_count++;
}
