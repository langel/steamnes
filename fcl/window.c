int window_has_focus = 0;

SDL_Rect window_rect;
SDL_Window * window;

// XXX WINDOW STATE - need a FCL State handler

#define window_file_ext ".cfg"
char window_state_file[256];

void window_state_set_filename(char * filename) {
	// xxx should set complete path
	strcpy(window_state_file, filename);
}

void window_state_load() {
	// XXX somewhere needs to check pos/size is visible on system
	//int display_count = SDL_GetNumVideoDisplays(window);
	//int display_current = SDL_GetWindowDisplayIndex(window);
	//SDL_Rect display_bounds = SDL_GetDisplayBounds(display_current, display_bounds);
	char * path = str_new(256);
	snprintf(path, 256, "%s%s", window_state_file, window_file_ext);
	//printf("%s\n", path);
	SDL_Rect rect;
	struct stat buffer;
	if (stat(path, &buffer) != 0) {
		rect = (SDL_Rect) { 100, 100, 1024, 576 };
	}
	else {
		FILE * fp = fopen(path, "r");
		fread(&rect, sizeof(struct SDL_Rect), 1, fp);
		fclose(fp);
	}
	//printf("window position: %d x %d\n", rect.x, rect.y);
	//printf("window dimensions: %d x %d\n", rect.w, rect.h);
	SDL_SetWindowSize(window, rect.w, rect.h);
	SDL_SetWindowPosition(window, rect.x, rect.y);
}

void window_state_save() {
	SDL_Rect rect;
	SDL_GetWindowPosition(window, &rect.x, &rect.y);
	// XXX struggling to remember window size
	SDL_GetRendererOutputSize(SDL_GetRenderer(window), &rect.w, &rect.h);
//	SDL_GetWindowSize(window, &rect.w, &rect.h);
	//printf("window position: %d x %d\n", rect.x, rect.y);
	//printf("window dimensions: %d x %d\n", rect.w, rect.h);
	char * path = str_new(256);
	snprintf(path, 256, "%s%s", window_state_file, window_file_ext);
	printf("%s\n", path);
	FILE * fp = fopen(path, "w");
	fwrite(&rect, sizeof(struct SDL_Rect), 1, fp);
	fclose(fp);
}


// HELPERS

// from here : https://en.wikipedia.org/wiki/16:9_aspect_ratio#Common_resolutions
uint16_t fcl_window_common_widths[16] = { 256, 426, 640, 848, 854, 960, 1024, 1280, 1366, 1600, 1920, 2560, 3200, 3840, 5120, 7680 };
uint16_t fcl_window_common_heights[16] = { 144, 240, 360, 480, 480, 540, 576, 720, 768, 900, 1080, 1440, 1800, 2160, 2880, 4320 };
void fcl_window_set_position(int pos_x, int pos_y) {
	SDL_SetWindowPosition(window, pos_x, pos_y);
}

/*
void fcl_window_center() {
	int pos_x = fcl_window_display_bounds.x + (fcl_window_display_bounds.w - fcl_window_width) / 2;
	int pos_y = fcl_window_display_bounds.y + (fcl_window_display_bounds.h - fcl_window_height) / 2;
	fcl_window_set_position(pos_x, pos_y);
}
*/

// focus window
void window_focus() {
	// XXX seems to be confusion here
	SDL_SetWindowAlwaysOnTop(window, SDL_TRUE);
//	SDL_SetWindowAlwaysOnTop(window, SDL_FALSE);
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


// CORE 

void window_init(char * name) {
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
	// default window start size?
	window = SDL_CreateWindow(name, window_rect.x, window_rect.y, window_rect.w, window_rect.h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	window_state_load();
}

void window_event(SDL_Event event) {
	if (event.type == SDL_WINDOWEVENT) {
		if (event.window.event == SDL_WINDOWEVENT_MOVED) {
			window_state_save();
			// xxx do we need to even track this?
			window_rect.x = event.window.data1;
			window_rect.y = event.window.data2;
		}
		if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
			window_state_save();
			// xxx do we need to even track this?
			window_rect.w = event.window.data1;
			window_rect.h = event.window.data2;
		}
	}
}

void window_update() {
	int flags = SDL_GetWindowFlags(window);
	window_has_focus = flags & SDL_WINDOW_INPUT_FOCUS;
}
