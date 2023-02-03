/*
	Central Processing Unit
	cpu.c
*/


const uint16_t cpu_vector_nmi = 0xfffa;
const uint16_t cpu_vector_reset = 0xfffc;
const uint16_t cpu_vecotr_irq = 0xfffe;

uint32_t cpu_cycle_count;

uint8_t cpu_a;
uint8_t cpu_x;
uint8_t cpu_y;
uint8_t cpu_sp;
uint16_t cpu_pc;
uint8_t cpu_flags;

void cpu_reset() {
	cpu_cycle_count = 0;
}
