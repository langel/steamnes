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
	nes_nmi = nes_irq = 0;
	cpu_countdowner = 0;
	ppu_countdowner = 0;
	debug_out(3, "CPU Reset Vector: 0x%4X", cpu_addr_reset);
	debug_out(3, "CPU IRQ Vector:   0x%4X", cpu_addr_irq);
	debug_out(3, "CPU NMI Vector:   0x%4X", cpu_addr_nmi);
}

void mbu_run() {
	while (mbu_running) {
		apu_clock();
		if (!cpu_countdowner) {
			cpu_countdowner = cpu_clock_div;
//			if (nes_nmi) debug_out(3, "nmi troubles");
			if (nes_nmi && (ppu_ctrl & 0x80)) cpu_nmi();
			else if (nes_irq) cpu_irq();
			else cpu_cycle();
		}
		if (cpu_write && cpu_bus == 0x55) {
			debug_out(3, "WRITE TO state-render-addr 0x%2X at 0x%4X", cpu_a, cpu_pw);
		}
		//if (cpu_pw == 0xc65a) nes_running = 0;
		//if (cpu_pw == 0xc14b) cpu_crash(0xff, 0);
		//if (cpu_pw == 0xc666) nes_running = 0;
		if (cpu_read | cpu_write) {
			if (cpu_bus = 0x2002) {

			}
			cpu_read = 0;
		}
		if (cpu_write) {
			//if (cpu_read) debug_out(3, "cpu r  0x%4x", cpu_bus);
			//if (cpu_write) debug_out(3, "cpu w  0x%4x", cpu_bus);
			mbu_data_bus = cpu_addr[cpu_bus];
			if (cpu_bus == 0x2000) {
				ppu_ctrl = cpu_a;
				debug_out(3, "PPU_CTRL SET: 0x%2X @ 0x%4x", ppu_ctrl, cpu_pw);
			}
			if (cpu_bus == 0x2001) ppu_mask = cpu_a;
			/*
			if ((cpu_bus & 0x3000) && !(cpu_bus & 0xc000)) {
				(cpu_read) ? ppu_read_reg(cpu_bus) : ppu_write_reg(cpu_bus);
			}
			*/
			// OAMDMA
			if (cpu_bus == 0x4014) {
				memcpy(&ppu_oam, &cpu_addr[cpu_a << 8], 0x100);
				cpu_cl = 513;
				debug_out(3, "oam dma");
			}
			cpu_write = 0;
		}
		if (!ppu_countdowner) {
			ppu_countdowner = ppu_clock_div;
			ppu_dot();
		}
		if (ppu_write) {
		}
		//cpu_nmi = ppu_int;
		cpu_countdowner--;
		ppu_countdowner--;
		mbu_cycle_count++;
		if (!nes_running) {
			// 0x800 covers 2kb of RAM
			for (int i = 0; i < 0x200; i += 16) {
				if (i % 256 == 0) debug_out(3, "CPU RAM PAGE %2X", i >> 8);
				debug_out(3, "%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x", 
				cpu_addr[i], cpu_addr[i+1], cpu_addr[i+2], cpu_addr[i+3], cpu_addr[i+4], cpu_addr[i+5], cpu_addr[i+6], cpu_addr[i+7], cpu_addr[i+8],
				cpu_addr[i+9], cpu_addr[i+10], cpu_addr[i+11], cpu_addr[i+12], cpu_addr[i+13], cpu_addr[i+14], cpu_addr[i+15]);
			}
			/*
			for (int i = 0; i < 0x400; i += 16) {
				if (i % 256 == 0) debug_out(3, "PPU RAM PAGE %2X", i >> 8);
				debug_out(3, "%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x", 
				ppu_ram[i], ppu_ram[i+1], ppu_ram[i+2], ppu_ram[i+3], ppu_ram[i+4], ppu_ram[i+5], ppu_ram[i+6], ppu_ram[i+7], ppu_ram[i+8],
				ppu_ram[i+9], ppu_ram[i+10], ppu_ram[i+11], ppu_ram[i+12], ppu_ram[i+13], ppu_ram[i+14], ppu_ram[i+15]);
			}
			*/
			for (int i = 0; i < 0x100; i += 16) {
				if (i % 256 == 0) debug_out(3, "PPU OAM PAGE %2X", i >> 8);
				debug_out(3, "%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x", 
				ppu_oam[i], ppu_oam[i+1], ppu_oam[i+2], ppu_oam[i+3], ppu_oam[i+4], ppu_oam[i+5], ppu_oam[i+6], ppu_oam[i+7], ppu_oam[i+8],
				ppu_oam[i+9], ppu_oam[i+10], ppu_oam[i+11], ppu_oam[i+12], ppu_oam[i+13], ppu_oam[i+14], ppu_oam[i+15]);
			}
			return;
		}
	}
}
