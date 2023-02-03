/*
	Central Processing Unit
	cpu.c
*/


const uint16_t cpu_vector_nmi = 0xfffa;
const uint16_t cpu_vector_reset = 0xfffc;
const uint16_t cpu_vector_irq = 0xfffe;

const int cpu_clock_div_ntsc = 12;
const int cpu_clock_div_pal = 16;
const int cpu_clock_div_dendy = 15;

uint32_t cpu_cycle_count;
int cpu_clock_div;

uint8_t cpu_a;
uint8_t cpu_x;
uint8_t cpu_y;
uint8_t cpu_sp;
uint16_t cpu_pc;
uint8_t cpu_flags;
uint16_t cpu_addr_bus;
uint8_t cpu_data_bus;
int pin_irq;
int pin_nmi;
int pin_rw;

void cpu_reset() {
	cpu_cycle_count = 0;
	cpu_clock_div = cpu_clock_div_ntsc;
}

void cpu_cycle() {
}
