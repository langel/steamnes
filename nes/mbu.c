/*
	Mother Board Unit
	mbu.c
*/


int mbu_clock_count;
int mbu_running = 0;

void mbu_start() {
	mbu_clock_count = 0;
	cpu_reset();
	ppu_reset();
	mbu_running = 1;
}

void mbu_run() {
	while (mbu_running) {
		apu_clock();
		if (mbu_clock_count % cpu_clock_div == 0) cpu_cycle();
		if (mbu_clock_count % ppu_clock_div == 0) ppu_dot();
		cpu_nmi = ppu_int;
		mbu_clock_count++;
	}
}
