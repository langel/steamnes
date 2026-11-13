

const uint8_t controller_a      = 0x01;
const uint8_t controller_b      = 0x02;
const uint8_t controller_select = 0x04;
const uint8_t controller_start  = 0x08;
const uint8_t controller_up     = 0x10;
const uint8_t controller_down   = 0x20;
const uint8_t controller_left   = 0x40;
const uint8_t controller_right  = 0x80;

uint8_t controller_1;
uint8_t controller_2;

SDL_GameController* controller = NULL;


void port_init() {
	for (int i = 0; i < SDL_NumJoysticks(); ++i) {
		if (SDL_IsGameController(i)) {
			controller = SDL_GameControllerOpen(i);
			debug_out(3, "%d", i);
			if (controller) break;
		}
	}
}

void port_latch(uint8_t byte) {
	if (byte) {
		controller_1 = 0;
		if (keys[SDL_SCANCODE_W] || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP)) {
			controller_1 |= controller_up;
		}
		if (keys[SDL_SCANCODE_S] || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
			controller_1 |= controller_down;
		}
		if (keys[SDL_SCANCODE_A] || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
			controller_1 |= controller_left;
		}
		if (keys[SDL_SCANCODE_D] || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
			controller_1 |= controller_right;
		}
		if (keys[SDL_SCANCODE_SEMICOLON] || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B)) {
			controller_1 |= controller_b;
		}
		if (keys[SDL_SCANCODE_APOSTROPHE] || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) {
			controller_1 |= controller_a;
		}
		if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X)) {
			controller_1 |= controller_b;
		}
		if (keys[SDL_SCANCODE_LEFTBRACKET] || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK)) {
			controller_1 |= controller_select;
		}
		if (keys[SDL_SCANCODE_RIGHTBRACKET] || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START)) {
			controller_1 |= controller_start;
		}
		//debug_out(3, "%8b", controller_1);
	}
}

void port_bitbang() {
	controller_1 >>= 1;
}

uint8_t port_peek() {
	return controller_1 & 0x01;
}
