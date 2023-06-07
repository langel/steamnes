

#define window_file_ext ".cfg"

// focus window
void window_focus(SDL_Window * window) {
	// XXX seems to be confusion here
	SDL_SetWindowAlwaysOnTop(window, SDL_TRUE);
	SDL_SetWindowAlwaysOnTop(window, SDL_FALSE);
	SDL_RaiseWindow(window);
}
// force window on top
void window_ontop_set_true(SDL_Window * window) {
	SDL_SetWindowAlwaysOnTop(window, SDL_TRUE);
}
// disable force window top
void window_ontop_set_false(SDL_Window * window) {
	SDL_SetWindowAlwaysOnTop(window, SDL_FALSE);
}


void window_state_load(SDL_Window * window, char * filename) {
	// XXX somewhere needs to check pos/size is visible on system
	//int display_count = SDL_GetNumVideoDisplays(window);
	//int display_current = SDL_GetWindowDisplayIndex(window);
	//SDL_Rect display_bounds = SDL_GetDisplayBounds(display_current, display_bounds);
	char * path = str_new(256);
	snprintf(path, 256, "%s%s", filename, window_file_ext);
	//printf("%s\n", path);
	struct stat buffer;
	if (stat(path, &buffer) != 0) return;
	SDL_Rect rect;
	FILE * fp = fopen(path, "r");
	fread(&rect, sizeof(struct SDL_Rect), 1, fp);
	fclose(fp);
	//printf("window position: %d x %d\n", rect.x, rect.y);
	//printf("window dimensions: %d x %d\n", rect.w, rect.h);
	SDL_SetWindowSize(window, rect.w, rect.h);
	SDL_SetWindowPosition(window, rect.x, rect.y);
}

void window_state_save(SDL_Window * window, char * filename) {
	SDL_Rect rect;
	SDL_GetWindowPosition(window, &rect.x, &rect.y);
	// XXX struggling to remember window size
	SDL_GetRendererOutputSize(SDL_GetRenderer(window), &rect.w, &rect.h);
//	SDL_GetWindowSize(window, &rect.w, &rect.h);
	//printf("window position: %d x %d\n", rect.x, rect.y);
	//printf("window dimensions: %d x %d\n", rect.w, rect.h);
	char * path = str_new(256);
	snprintf(path, 256, "%s%s", filename, window_file_ext);
	printf("%s\n", path);
	FILE * fp = fopen(path, "w");
	fwrite(&rect, sizeof(struct SDL_Rect), 1, fp);
	fclose(fp);
}

// from here : https://en.wikipedia.org/wiki/16:9_aspect_ratio#Common_resolutions
uint16_t fcl_window_common_widths[16] = { 256, 426, 640, 848, 854, 960, 1024, 1280, 1366, 1600, 1920, 2560, 3200, 3840, 5120, 7680 };
uint16_t fcl_window_common_heights[16] = { 144, 240, 360, 480, 480, 540, 576, 720, 768, 900, 1080, 1440, 1800, 2160, 2880, 4320 };

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

void fcl_window_set_position(int pos_x, int pos_y) {
	SDL_SetWindowPosition(fcl_window_object, pos_x, pos_y);
}

void fcl_window_center() {
	int pos_x = fcl_window_display_bounds.x + (fcl_window_display_bounds.w - fcl_window_width) / 2;
	int pos_y = fcl_window_display_bounds.y + (fcl_window_display_bounds.h - fcl_window_height) / 2;
	fcl_window_set_position(pos_x, pos_y);
}
