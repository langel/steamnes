

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


void port_init() {
	controller_1 = 0;
}

void port_latch(uint8_t byte) {
	if (byte) {
		controller_1 = 0;
		if (keys[SDL_SCANCODE_W]) controller_1 |= controller_up;
		if (keys[SDL_SCANCODE_S]) controller_1 |= controller_down;
		if (keys[SDL_SCANCODE_A]) controller_1 |= controller_left;
		if (keys[SDL_SCANCODE_D]) controller_1 |= controller_right;
		if (keys[SDL_SCANCODE_SEMICOLON]) controller_1 |= controller_b;
		if (keys[SDL_SCANCODE_APOSTROPHE]) controller_1 |= controller_a;
		if (keys[SDL_SCANCODE_LEFTBRACKET]) controller_1 |= controller_select;
		if (keys[SDL_SCANCODE_RIGHTBRACKET]) controller_1 |= controller_start;
		//debug_out(3, "%8b", controller_1);
	}
}

int port_bitbang() {
	int bit = controller_1 & 0x01;
	controller_1 >>= 1;
	return bit;
}
