/*
	Central Processing Unit
	cpu.c
*/

uint8_t cpu_addr[0x10000] = { 0 };

#define cpu_vector_nmi = 0xfffa;
#define cpu_vector_reset = 0xfffc;
#define cpu_vector_irq = 0xfffe;

#define cpu_clock_div_ntsc = 12;
#define cpu_clock_div_pal = 16;
#define cpu_clock_div_dendy = 15;

int cpu_clock_div;
uint32_t cpu_cycle_count;
uint16_t cpu_addr_bus;
uint8_t cpu_data_bus;

uint8_t cpu_a; // accumulator
uint8_t cpu_x; // x register
uint8_t cpu_y; // y register
uint8_t cpu_p; // processor status
uint8_t cpu_s; // stack pointer
uint8_t cpu_ph; // program counter high
uint8_t cpu_pl; // program counter low
uint16_t cpu_pw; // program counter word

#define cpu_fn 0x01 // negative 
#define cpu_fv 0x02 // overflow
#define cpu_fx 0x04 // "reserved"
#define cpu_fb 0x08 // break
#define cpu_fd 0x10 // decimal
#define cpu_fi 0x20 // interupt disable
#define cpu_fz 0x40 // zero
#define cpu_fc 0x80 // carry

// pins
int cpu_irq;
int cpu_nmi;
int cpu_rw;

void cpu_reset() {
	cpu_cycle_count = 0;
	cpu_clock_div = cpu_clock_div_ntsc;
}

/*
void cpu_irq() {
}

void cpu_nmi() {
}
*/

void cpu_cycle() {
}
