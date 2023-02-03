/*
	Mother Board Unit
	mbu.c
*/


uint64_t mbu_clock_count;

void mbu_start() {
	mbu_clock_count = 0;
	cpu_reset();
	ppu_reset();
}
