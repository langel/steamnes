/*
	Central Processing Unit
	cpu.c
*/

uint8_t cpu_addr[0x10000] = { 0 };

#define cpu_vector_nmi   0xfffa
#define cpu_vector_reset 0xfffc
#define cpu_vector_irq   0xfffe
int cpu_addr_nmi;
int cpu_addr_reset;
int cpu_addr_irq;

#define cpu_clock_div_ntsc  12
#define cpu_clock_div_pal   16
#define cpu_clock_div_dendy 15

int      cpu_clock_div;
uint32_t cpu_cycle_count;
uint16_t cpu_addr_bus;

uint8_t  cpu_a;  // accumulator
uint8_t  cpu_x;  // x register
uint8_t  cpu_y;  // y register
uint8_t  cpu_p;  // processor status
uint8_t  cpu_s;  // stack pointer
uint8_t  cpu_ph; // program counter high
uint8_t  cpu_pl; // program counter low
uint16_t cpu_pw; // program counter word
uint8_t  cpu_cl; // program cycle downcounter before next

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
int cpu_read;
int cpu_write;

void cpu_reset() {
	cpu_cycle_count = 0;
	cpu_read = cpu_write = 0;
	cpu_clock_div = cpu_clock_div_ntsc;
	cpu_addr_irq = cpu_addr[cpu_vector_irq] + (cpu_addr[cpu_vector_irq + 1] << 8);
	cpu_addr_reset = cpu_addr[cpu_vector_reset] + (cpu_addr[cpu_vector_reset + 1] << 8);
	cpu_addr_nmi = cpu_addr[cpu_vector_nmi] + (cpu_addr[cpu_vector_nmi + 1] << 8);
	cpu_pw = cpu_addr_reset;
}

/*
void cpu_irq() {
}

void cpu_nmi() {
}
*/
// helper functions
#define cpu_addr_load_abs() { cpu_pw++; cpu_addr_bus = cpu_addr[cpu_pw]; cpu_pw++; cpu_addr_bus += (cpu_addr[cpu_pw] << 8); cpu_pw++; }
#define cpu_addr_load_zpg() { cpu_pw++; cpu_addr_bus = cpu_addr[cpu_pw]; cpu_pw++; }
#define cpu_addr_load_zpx() { cpu_pw++; cpu_addr_bus = cpu_addr[(cpu_pw + cpu_x) & 0xff]; cpu_pw++; }
#define cpu_bit_op(byte) { cpu_p = 0xff - (cpu_fn | cpu_fv | cpu_fz); if (byte & cpu_fn) cpu_p |= cpu_fn; if (byte & cpu_fv) cpu_p |= cpu_fv; if (!(byte & cpu_a)) cpu_p |= cpu_fz; }
#define cpu_p_set_nz(byte) { cpu_p &= ~(cpu_fn | cpu_fz); cpu_p |= ((byte) & cpu_fn) | ((byte) ? 0 : cpu_fz); cpu_pw++; }
#define cpu_push(byte) { cpu_addr[0x100 | cpu_s] = byte; cpu_s--; }

// status registers
#define clc_op() { cpu_p &= ~cpu_fc; cpu_pw++; cpu_cl = 2; }
#define cld_op() { cpu_p &= ~cpu_fd; cpu_pw++; cpu_cl = 2; }
#define cli_op() { cpu_p &= ~cpu_fi; cpu_pw++; cpu_cl = 2; }
#define clv_op() { cpu_p &= ~cpu_fv; cpu_pw++; cpu_cl = 2; }
#define sec_op() { cpu_p |=  cpu_fc; cpu_pw++; cpu_cl = 2; }
#define sed_op() { cpu_p |=  cpu_fd; cpu_pw++; cpu_cl = 2; }
#define sei_op() { cpu_p |=  cpu_fi; cpu_pw++; cpu_cl = 2; }
// transfer registers
#define tax_op() { cpu_x = cpu_a; cpu_p_set_nz(cpu_x); cpu_cl = 2; }
#define tay_op() { cpu_y = cpu_a; cpu_p_set_nz(cpu_y); cpu_cl = 2; }
#define tsx_op() { cpu_x = cpu_s; cpu_p_set_nz(cpu_x); cpu_cl = 2; }
#define txa_op() { cpu_a = cpu_x; cpu_p_set_nz(cpu_a); cpu_cl = 2; }
#define txs_op() { cpu_s = cpu_x; cpu_pw++;            cpu_cl = 2; }
#define tya_op() { cpu_a = cpu_y; cpu_p_set_nz(cpu_a); cpu_cl = 2; }
// decrementors and incrementors
#define dex_op() { cpu_x--; cpu_p_set_nz(cpu_x); cpu_cl = 2; }
#define dey_op() { cpu_y--; cpu_p_set_nz(cpu_y); cpu_cl = 2; }
#define inx_op() { cpu_x++; cpu_p_set_nz(cpu_x); cpu_cl = 2; }
#define iny_op() { cpu_y++; cpu_p_set_nz(cpu_y); cpu_cl = 2; }
// weirdos
#define bit_abs() { cpu_addr_load_abs(); cpu_bit_op(cpu_addr[cpu_addr_bus]); cpu_cl = 4; }
// branches
#define jsr_op()  { cpu_addr_load_abs(); cpu_pw = cpu_addr_bus; cpu_push(cpu_addr_bus >> 8); cpu_push(cpu_addr_bus & 0xff); cpu_cl = 6; }
// comparers
#define cpx_imm() { cpu_pw++; cpu_p &= ~cpu_fc; if (cpu_x >= cpu_addr[cpu_pw]) cpu_p |= cpu_fc; cpu_p_set_nz((cpu_x - cpu_addr[cpu_pw]) & 0xff); }
// loaders
#define lda_imm() { cpu_pw++; cpu_a = cpu_addr[cpu_pw]; cpu_p_set_nz(cpu_x); cpu_read++; cpu_cl = 2; }
#define ldx_imm() { cpu_pw++; cpu_x = cpu_addr[cpu_pw]; cpu_p_set_nz(cpu_x); cpu_read++; cpu_cl = 2; }
#define ldy_imm() { cpu_pw++; cpu_y = cpu_addr[cpu_pw]; cpu_p_set_nz(cpu_y); cpu_read++; cpu_cl = 2; }
// storers
#define sta_abs() { cpu_addr_load_abs(); cpu_addr[cpu_addr_bus] = cpu_a; cpu_write++; cpu_cl = 4; }
#define stx_abs() { cpu_addr_load_abs(); cpu_addr[cpu_addr_bus] = cpu_x; cpu_write++; cpu_cl = 4; }
#define sty_abs() { cpu_addr_load_abs(); cpu_addr[cpu_addr_bus] = cpu_y; cpu_write++; cpu_cl = 4; }
#define sta_zpx() { cpu_addr_load_zpx(); cpu_addr[cpu_addr_bus] = cpu_x; cpu_write++; cpu_cl = 4; }

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
		// decrementors and incrementors
		case 0xca: dex_op(); break;
		case 0x88: dey_op(); break;
		case 0xe8: inx_op(); break;
		case 0xc8: iny_op(); break;
		// weirdos
		case 0x2c: bit_abs(); break;
		// branches
		case 0x20: jsr_op(); break;
		// comparers
		case 0xe0: cpx_imm(); break;
		// loaders
		case 0xa9: lda_imm(); break;
		case 0xa2: ldx_imm(); break;
		case 0xa0: ldy_imm(); break;
		// storers
		case 0x8d: sta_abs(); break;
		case 0x95: sta_zpx(); break;
		case 0x8e: stx_abs(); break;
		case 0x8c: stx_abs(); break;
		default:
			debug_out(1, "PROCESSOR DISCOMBOBULATION");
			debug_out(1, "2a03 undefined opcode: 0x%2X", opcode);
			debug_out(3, "program counter pos: 0x%4X", cpu_pw);
			debug_out(3, "A: %2x  X: %2x  Y: %2x  S: %2x", cpu_a, cpu_x, cpu_y, cpu_s);
			debug_out(3, "C: %d  Z: %d  I: %d  D: %d  B: %d  V: %d  N: %d", 
				(cpu_p & cpu_fc) ? 1 : 0,
				(cpu_p & cpu_fz) ? 1 : 0,
				(cpu_p & cpu_fi) ? 1 : 0,
				(cpu_p & cpu_fd) ? 1 : 0,
				(cpu_p & cpu_fb) ? 1 : 0,
				(cpu_p & cpu_fv) ? 1 : 0,
				(cpu_p & cpu_fn) ? 1 : 0
			);
			debug_out(3, "CPU cycles: %d", cpu_cycle_count);
			debug_out(3, "PPU cycles: %d", ppu_cycle_count);
			debug_out(3, "CLK cycles: %d", mbu_cycle_count);
			debug_out(0, "NEFARIOUS CRASH EXIT");
			nes_running = 0;
	}
}
