


#define fvc_max_pixels 262144 // * 4 for 32bit color
//uint8_t *fvc_ram[fvc_max_pixels];
uint8_t *fvc_ram;
uint16_t fvc_width = 420;
uint16_t fvc_height= 200;
SDL_Texture *fvc_texture;
SDL_Texture *fvc_overscale;
SDL_Color fvc_colors[256];
SDL_Renderer *fvc_renderer;
SDL_Surface *fvc_surface;

void fvc_clear(uint8_t index) {
	memset(fvc_ram, index, fvc_max_pixels * sizeof(uint32_t));
	//SDL_RenderFillRect(renderer, &window_rect);
}

void fvc_frame_next() {
	/* surface based code 
	// blit that shit
	//fvc_clear(0);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(fvc_renderer, fvc_surface);
	SDL_RenderCopy(fvc_renderer, texture, NULL, NULL);
	SDL_RenderPresent(fvc_renderer);
	SDL_DestroyTexture(texture);
	*/
	//frame_wait_next();
	// "shader effects" xD
	SDL_SetRenderTarget(fvc_renderer, fvc_overscale);
	SDL_RenderCopy(fvc_renderer, fvc_texture, NULL, NULL);
	SDL_SetRenderTarget(fvc_renderer, NULL);
	SDL_RenderCopy(fvc_renderer, fvc_overscale, NULL, NULL);
	SDL_RenderPresent(fvc_renderer);
}

void fvc_set_dimensions(int x, int y) {
	fvc_width = x;
	fvc_height = y;
}

SDL_Texture * fvc_texture_create(int width, int height) {
	return SDL_CreateTexture(fvc_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
}

void fvc_init(int x, int y) {
	fvc_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	fvc_set_dimensions(x, y);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	fvc_texture = SDL_CreateTexture(fvc_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, fvc_width, fvc_height);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
	fvc_overscale = SDL_CreateTexture(fvc_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, fvc_width * 3, fvc_height * 3);
	//fvc_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, fvc_width, fvc_height, 8, 0, 0, 0, 0);
	//SDL_SetPaletteColors(fvc_surface->format->palette, fvc_colors, 0, 256);
	//fvc_ram = (uint8_t) fvc_surface->pixels;
	//fvc_clear(0);
	SDL_SetRenderDrawBlendMode(fvc_renderer, SDL_BLENDMODE_BLEND);
}

void fvc_set_color_index(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
	fvc_colors[index].r = r;
	fvc_colors[index].g = g;
	fvc_colors[index].b = b;
	fvc_colors[index].a = 255;
}

void fvc_set_draw_color(uint8_t index) {
	SDL_SetRenderDrawColor(fvc_renderer, fvc_colors[index].r, fvc_colors[index].g, fvc_colors[index].b, 255);
}

void fvc_set_pixel(int x, int y, uint8_t index) {
	uint8_t* fvc_ram = fvc_surface->pixels;
	fvc_ram[y * fvc_width + x] = index;
}
