



SDL_Window *fcl_window_object;
SDL_Rect fcl_window_display_rect;
SDL_Rect fcl_window_display_bounds;
uint8_t fcl_window_display_count;

uint16_t fcl_window_width;
uint16_t fcl_window_height;
int16_t fcl_window_pos_x;
int16_t fcl_window_pos_x_max;
int16_t fcl_window_pos_x_min;
int16_t fcl_window_pos_y;
int16_t fcl_window_pos_y_max;
int16_t fcl_window_pos_y_min;

void fcl_window_init() {
	fcl_window_display_count = SDL_GetNumVideoDisplays();
	// XXX temporarily use last display
	SDL_GetDisplayUsableBounds(fcl_window_display_count - 1, &fcl_window_display_bounds);
}

void fcl_window_create(const char *title, uint16_t width, uint16_t height) {
	fcl_window_object = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
	int pos_x, pos_y;
	SDL_GetWindowPosition(fcl_window_object, &pos_x, &pos_y);
	// width stuff
	fcl_window_width = width;
	fcl_window_pos_x = pos_x;
	fcl_window_pos_x_min = fcl_window_display_bounds.x;
	fcl_window_pos_x_max = fcl_window_display_bounds.x + fcl_window_display_bounds.w - fcl_window_width;
	// height stuff
	fcl_window_height = height;
	fcl_window_pos_y = pos_y;
	fcl_window_pos_y_min = fcl_window_display_bounds.y;
	fcl_window_pos_y_max = fcl_window_display_bounds.y + fcl_window_display_bounds.w - fcl_window_height;
}

