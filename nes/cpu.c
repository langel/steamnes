/*
	Central Processing Unit
	cpu.c
*/

uint8_t cpu_addr[0x10000] = { 0 };

#define cpu_vector_nmi   0xfffa
#define cpu_vector_reset 0xfffc
#define cpu_vector_irq   0xfffe

#define cpu_clock_div_ntsc  12
#define cpu_clock_div_pal   16
#define cpu_clock_div_dendy 15

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
uint8_t cpu_cl; // program cycle downcounter before next

#define cpu_fc 0x01 // carry
#define cpu_fz 0x02 // zero
#define cpu_fi 0x04 // interupt disable
#define cpu_fd 0x08 // decimal
#define cpu_fb 0x10 // break
#define cpu_fx 0x20 // "reserved"
#define cpu_fv 0x40 // overflow
#define cpu_fn 0x80 // negative 

// pins
int cpu_irq;
int cpu_nmi;
int cpu_rw;

void cpu_reset() {
	cpu_cycle_count = 0;
	cpu_clock_div = cpu_clock_div_ntsc;
	cpu_pw = cpu_addr[cpu_vector_reset] + (cpu_addr[cpu_vector_reset + 1] << 8);
}

/*
void cpu_irq() {
}

void cpu_nmi() {
}
*/
// helper functions
#define cpu_p_set_nz(byte) { cpu_p &= ~(cpu_fn | cpu_fz); cpu_p |= ((byte) & cpu_fn) | ((byte) ? 0 : cpu_fz); }

// status registers
#define clc_op() { cpu_p &= ~cpu_fc; cpu_pw++; cpu_cl = 2; }
#define cld_op() { cpu_p &= ~cpu_fd; cpu_pw++; cpu_cl = 2; }
#define cli_op() { cpu_p &= ~cpu_fi; cpu_pw++; cpu_cl = 2; }
#define clv_op() { cpu_p &= ~cpu_fv; cpu_pw++; cpu_cl = 2; }
#define sec_op() { cpu_p |=  cpu_fc; cpu_pw++; cpu_cl = 2; }
#define sed_op() { cpu_p |=  cpu_fd; cpu_pw++; cpu_cl = 2; }
#define sei_op() { cpu_p |=  cpu_fi; cpu_pw++; cpu_cl = 2; }
// transfer registers
#define tax_op() { cpu_x = cpu_a; cpu_p_set_nz(cpu_x); cpu_pw++; cpu_cl = 2; }
#define tay_op() { cpu_y = cpu_a; cpu_p_set_nz(cpu_y); cpu_pw++; cpu_cl = 2; }
#define tsx_op() { cpu_x = cpu_s; cpu_p_set_nz(cpu_x); cpu_pw++; cpu_cl = 2; }
#define txa_op() { cpu_a = cpu_x; cpu_p_set_nz(cpu_a); cpu_pw++; cpu_cl = 2; }
#define txs_op() { cpu_s = cpu_x;                      cpu_pw++; cpu_cl = 2; }
#define tya_op() { cpu_a = cpu_y; cpu_p_set_nz(cpu_a); cpu_pw++; cpu_cl = 2; }

void cpu_cycle() {
	if (cpu_cl) {
		cpu_cycle_count++;
		cpu_cl--;
		return;
	}
	uint8_t opcode = cpu_addr[cpu_pw];
	switch(opcode) {
		// status registers
		case 0x18: clc_op(); break;
		case 0xd8: cld_op(); break;
		case 0x58: cli_op(); break;
		case 0xb8: clv_op(); break;
		case 0x38: sec_op(); break;
		case 0x78: sei_op(); break;
		case 0xf8: sed_op(); break;
		// transfer registers
		case 0xaa: tax_op(); break;
		case 0xa8: tay_op(); break;
		case 0xba: tsx_op(); break;
		case 0x8a: txa_op(); break;
		case 0x9a: txs_op(); break;
		case 0x98: tya_op(); break;
	}
}
