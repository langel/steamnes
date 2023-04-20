/*
	Mother Board Unit
	mbu.c
*/


int mbu_clock_count;
int mbu_running = 0;

uint8_t mbu_data_bus;

void mbu_start() {
	mbu_clock_count = 0;
	cpu_reset();
	ppu_reset();
	mbu_running = 1;
	nes_running = 1;
	debug_out(3, "CPU Reset Vector: 0x%4X", cpu_addr_reset);
	debug_out(3, "CPU IRQ Vector:   0x%4X", cpu_addr_irq);
	debug_out(3, "CPU NMI Vector:   0x%4X", cpu_addr_nmi);
}

void mbu_run() {
	while (mbu_running) {
		apu_clock();
		if (!(mbu_clock_count % cpu_clock_div)) cpu_cycle();
		if (cpu_read | cpu_write) {
			mbu_data_bus = cpu_addr[cpu_addr_bus];
			if ((cpu_addr_bus & 0x3000) && !(cpu_addr_bus & 0xc000)) {
				(cpu_read) ? ppu_read_reg(cpu_addr_bus) : ppu_write_reg(cpu_addr_bus);
			}
			cpu_read = 0;
			cpu_write = 0;
		}
		if (!(mbu_clock_count % ppu_clock_div)) ppu_dot();
		if (ppu_write) {
		}
		cpu_nmi = ppu_int;
		mbu_cycle_count++;
		if (!nes_running) return;
	}
}
