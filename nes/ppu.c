/*
	Picture Processing Unit
	ppu.c
*/

uint8_t ppu_addr[0x4000] = { 0 };
uint8_t ppu_oam[0x0100] = { 0 };

typedef struct {
	uint8_t id;
	uint8_t pattern0;
	uint8_t pattern1;
	uint8_t attributes;
	uint8_t x_pos;
	uint8_t y_pos;
} ppu_line_sprite;
ppu_line_sprite ppu_line_sprites[8];

#define ppu_clock_div_ntsc  4
#define ppu_clock_div_pal   5
#define ppu_clock_div_dendy 5

int ppu_dot_frame_max = 89341;
int ppu_clock_div;
int ppu_write;
int ppu_nt_arrangement = 0;
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
	//dpipe_init("nes/ppu.c");
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
//		ppu_dot_data[i] = nes_pal[ppu_addr[i] & 0x3f];
	}
	// XXX debug palettes
	for (int i = 0; i < 32; i += 2) {
		ppu_dot_data[i*256] = nes_pal[ppu_addr[0x3f00 + i]];
		ppu_dot_data[(i+1)*256] = nes_pal[ppu_addr[0x3f00 + i]];
	}
	SDL_UpdateTexture(ppu_dot_texture, NULL, ppu_dot_data, 256 * 4);
	SDL_RenderCopy(fvc_renderer, ppu_dot_texture, NULL, &(SDL_Rect) { 47, 0, 256, 240 });
	dpipe_update();
}

void ppu_sprite_evaluation(int scanline) {
	// clear scanline buffer
	memset(ppu_line_sprites, 0x00, sizeof ppu_line_sprites);
	int n = 0;
	// iterate over sprite oam data
	for (int i = 0; i < 256; i += 4) {
		int row = scanline - ppu_oam[i];
		if (row >= 0 && row < 8 && n < 8) {
			int attr = ppu_oam[i + 2];
			if (attr & 0x80) row = 7 - row;
			int base = ((ppu_ctrl & 0x08) ? 0x1000 : 0x0000) + (ppu_oam[i + 1] << 4) + row;
			ppu_line_sprites[n] = (ppu_line_sprite) { 
				i,
				ppu_addr[base], 
				ppu_addr[base+8], 
				attr, 
				ppu_oam[i + 3], 
				ppu_oam[i + 0]
			};
			n++;
		}
		if (n >= 8) break;
	}
}

void ppu_dot() {
	static int pxl_pos;
	if (ppu_scanline == 0 && ppu_scan_dot == 0) {
		// reset sprite 0 and sprite overflow bits
		ppu_status &= (uint8_t) ~0x60;
		pxl_pos = 0;
	}
	if (ppu_scanline < 240) {
		// render space
		if (ppu_scan_dot < 256) {
			int bg_color = 0;
			int spr_color = 0;
			// BACKGROUND RENDER
			// pattern table
			int pt = (ppu_ctrl & 0x10) ? 0x1000 : 0x0000;
			// scroll position
			int pos_x, pos_x_hi;
			pos_x = ppu_scroll_x + ppu_scan_dot;
			pos_x_hi = (pos_x >> 8) & 1;
			pos_x %= 256;
			int pos_y, pos_y_hi;
			pos_y = ppu_scroll_y + ppu_scanline;
			pos_y_hi = (pos_y >> 8) & 1;
			pos_y %= 240;
			// name table
			int nt = ppu_ctrl & 0x03;
			if (ppu_nt_arrangement) {
				// vertical arrangement
				nt = (nt & 1) ^ pos_x_hi;
			}
			else {
				// horizontal arrangement
				nt = (nt & 2) ^ (pos_y_hi << 1);
			}
			nt = 0x2000 + (nt << 10);
			// find current dot
			int tile_id = ppu_addr[nt + (pos_x >> 3) + ((pos_y >> 3) << 5)];
			int tile_x = pos_x & 0x07;
			int tile_col = 1 << (0x07 - tile_x);
			int tile_y = pos_y & 0x07;
			int tile_offset = pt + (tile_id << 4) + tile_y;
			int tile_row_lo = ppu_addr[tile_offset];
			int tile_row_hi = ppu_addr[tile_offset + 8];
			// attributes
			int tile_pal = 0;
			int attr = ppu_addr[nt + 0x3c0 + (pos_x >> 5) + ((pos_y >> 5) << 3)];
			int attr_loc = ((pos_x >> 4) & 1) | (((pos_y >> 4) & 1) << 1);
			if (attr_loc == 0) tile_pal = attr & 0x03;
			if (attr_loc == 1) tile_pal = (attr & 0x0c) >> 2;
			if (attr_loc == 2) tile_pal = (attr & 0x30) >> 4;
			if (attr_loc == 3) tile_pal = (attr & 0xc0) >> 6;
			bg_color = (tile_row_lo & tile_col) ? 1 : 0;
			bg_color += (tile_row_hi & tile_col) ? 2 : 0;
			bg_color += tile_pal << 2;

			// SPRITE(s) RENDER
			spr_color = 0;
			for (int i = 0; i < 8; i++) {
				ppu_line_sprite *spr = &ppu_line_sprites[i];
				int dx = (int) ppu_scan_dot - (int) spr->x_pos;
				if (dx < 0 || dx > 7) continue;
				int bit = (spr->attributes & 0x40) ? dx : 7 - dx;
				int temp_color = ((spr->pattern0 >> bit) & 1) | (((spr->pattern1 >> bit) & 1) << 1);
				if (temp_color) { 
					if (spr->id == 0 && temp_color) {
						ppu_status |= 0x40;
					}
					spr_color = temp_color + ((spr->attributes & 0x03) << 2) + 0x10;
				}
			}

			// DONE finding colors
			int pal_addr = (spr_color & 0x03) ? spr_color : bg_color;
			ppu_dot_data[pxl_pos] = nes_pal[ppu_addr[0x3f00 + pal_addr] & 0x3f];
			pxl_pos++;
		}
		// SPRITE EVALUATION
		if (ppu_scan_dot == 256) {
			ppu_sprite_evaluation(ppu_scanline);
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
