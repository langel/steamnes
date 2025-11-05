// gives program stdout access
//#define SDL_MAIN_HANDLED
#define SCREEN_W 350
#define SCREEN_H 240

#include "./fcl/core.c"
#include "./nes/core.c"
#include "./emu/core.c"

int frame_counter;

void core_init() {
	fcl_init();
	audio_init(32000, 2, 1024, AUDIO_F32SYS, &audio_callback);
	window_state_set_filename("snw");
	window_init("SteamNES");
	//dpipe_init("src/state_game_play.c");
	fvc_init(SCREEN_W, SCREEN_H);
	//font_init();
	//grafx_init();
	//map_grafx_init();
	//state_controller_init();
	//printf("init\n");
}

int main(int argc, char* args[]) {
	debug_output_target = all;
	debug_level = 4;
	debug_out(1, "SteamNES");
	//int error = rom_load("game_rom.nes");
	//int error = rom_load("roms//Baseball (J).nes"); illegal opcode 0x1a
	//int error = rom_load("roms//Burger Time (J).nes"); gray
	//int error = rom_load("roms//Dig Dug II (J) [!].nes"); // tiles
	//int error = rom_load("roms//Door Door (J).nes"); // barely anything
	//int error = rom_load("roms//Elevator Action (J).nes");
	int error = rom_load("roms//F-1 Race (J).nes"); // tiles
	//int error = rom_load("roms//Pac-Land (J) [a1].nes"); // mmc1
	//int error = rom_load("roms//Spartan X (J).nes"); // tiles
	//int error = rom_load("roms//Squoon (J) [!].nes");
	//int error = rom_load("roms//Super Mario Bros..nes"); // gray
	//int error = rom_load("roms//Thexder (J) [!].nes");
	//int error = rom_load("test_roms//01-implied.nes");
	//int error = rom_load("test_roms//02-immediate.nes");
	//int error = rom_load("test_roms//03-zero_page.nes");
	//int error = rom_load("test_roms//official_only.nes");
	if (error) {
		debug_out(0, "failed to load or identify rom file");
		return 0;
	}
	core_init();
	mbu_init();
	mbu_start();
	frame_set_fps(60);
	while (fcl_running && nes_running) {
		SDL_SetRenderTarget(fvc_renderer, fvc_texture);
		mbu_frame();
		frame_counter++;
		frame_wait_next();
		fcl_update();
		if (keys[SDL_SCANCODE_ESCAPE]) fcl_shutdown();
	}
	fcl_quit();
	return 0;
}
