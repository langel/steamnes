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

uint8_t  cpu_a;  // accumulator
uint8_t  cpu_x;  // x register
uint8_t  cpu_y;  // y register
uint8_t  cpu_p;  // processor status
uint8_t  cpu_s;  // stack pointer
uint8_t  cpu_ph; // program counter high
uint8_t  cpu_pl; // program counter low
uint16_t cpu_pw; // program counter word
uint16_t cpu_cl; // program cycle downcounter before next

uint16_t cpu_alu;  // arithmetic logic unit
uint16_t cpu_bus;  // current bus address
uint8_t  cpu_data; // inner temp

#define cpu_fc 0x01 // carry
#define cpu_fz 0x02 // zero
#define cpu_fi 0x04 // interupt disable
#define cpu_fd 0x08 // decimal
#define cpu_fb 0x10 // break
#define cpu_fx 0x20 // "reserved"
#define cpu_fv 0x40 // overflow
#define cpu_fn 0x80 // negative 

// pins
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

// helper functions
#define cpu_p_set_nz(byte) { cpu_p &= ~(cpu_fn | cpu_fz); cpu_p |= ((byte) & cpu_fn) | ((byte) ? 0 : cpu_fz); }
#define cpu_addr_load_abs() { cpu_pw++; cpu_bus = cpu_addr[cpu_pw]; cpu_pw++; cpu_bus += (cpu_addr[cpu_pw] << 8); cpu_pw++; }
#define cpu_addr_load_abx() { cpu_pw++; cpu_bus = cpu_addr[cpu_pw]; cpu_pw++; cpu_bus += (cpu_addr[cpu_pw] << 8) + cpu_x; cpu_pw++; }
#define cpu_addr_load_aby() { cpu_pw++; cpu_bus = cpu_addr[cpu_pw]; cpu_pw++; cpu_bus += (cpu_addr[cpu_pw] << 8) + cpu_y; cpu_pw++; }
#define cpu_addr_load_zpg() { cpu_pw++; cpu_bus = cpu_addr[cpu_pw]; cpu_pw++; }
#define cpu_addr_load_zpx() { cpu_pw++; cpu_bus = (cpu_addr[cpu_pw] + cpu_x) & 0xff; cpu_pw++; }
#define cpu_addr_load_zpy() { cpu_pw++; cpu_bus = (cpu_addr[cpu_pw] + cpu_y) & 0xff; cpu_pw++; }
#define cpu_addr_load_idx() { cpu_pw++; cpu_bus = cpu_addr[(cpu_pw + cpu_x) & 0xff]; cpu_bus += cpu_addr[(cpu_pw + 1 + cpu_x) && 0xff] << 8; cpu_pw++; }
#define cpu_addr_load_idy() { cpu_pw++; cpu_bus = cpu_addr[cpu_addr[cpu_pw]] + cpu_y; cpu_bus += (cpu_addr[(cpu_addr[cpu_pw] + 1) & 0xff] << 8); cpu_pw++; }
#define cpu_addr_load_ind() { cpu_pw++; cpu_bus = cpu_addr[cpu_pw]; cpu_pw++; cpu_bus += cpu_addr[cpu_pw] << 8; cpu_pw++; }
#define cpu_branch(byte) { cpu_pw++; if (byte) { if (cpu_addr[cpu_pw] & 0x80) cpu_pw -= cpu_addr[cpu_pw] ^ 0xff; else cpu_pw += cpu_addr[cpu_pw] + 1; } else cpu_pw++; }
#define cpu_op_bit(byte) { cpu_p = 0xff - (cpu_fn | cpu_fv | cpu_fz); if (byte & cpu_fn) cpu_p |= cpu_fn; if (byte & cpu_fv) cpu_p |= cpu_fv; if (!(byte & cpu_a)) cpu_p |= cpu_fz; }
#define cpu_op_adc(byte) { cpu_alu = cpu_a + byte + ((cpu_p & cpu_fc) ? 1 : 0); if (cpu_alu & 0xff00) cpu_p |= cpu_fc; else cpu_p &= ~cpu_fc; \
                           if ((cpu_a & 0x80) != (cpu_alu & 0x80)) cpu_p |= cpu_fv; else cpu_p &= ~cpu_fv; cpu_a = cpu_alu & 0xff; cpu_p_set_nz(cpu_a); }
#define cpu_op_sbc(byte) { cpu_alu = cpu_a - byte - ((cpu_p & cpu_fc) ? 0 : 1); if (cpu_alu & 0xff00) cpu_p &= ~cpu_fc; else cpu_p |= cpu_fc; \
                           if ((cpu_a & 0x80) != (cpu_alu & 0x80)) cpu_p |= cpu_fv; else cpu_p &= ~cpu_fv; cpu_a = cpu_alu & 0xff; cpu_p_set_nz(cpu_a); }
#define cpu_op_and(byte) { cpu_a &= byte; cpu_p_set_nz(byte); }
#define cpu_op_eor(byte) { cpu_a ^= byte; cpu_p_set_nz(byte); }
#define cpu_op_ora(byte) { cpu_a |= byte; cpu_p_set_nz(byte); }
#define cpu_op_asl(byte) { if (byte & 0x80) cpu_p |= cpu_fc; else cpu_p &= ~cpu_fc; byte <<= 1; cpu_p_set_nz(byte); }
#define cpu_op_lsr(byte) { if (byte & 0x01) cpu_p |= cpu_fc; else cpu_p &= ~cpu_fc; byte >>= 1; cpu_p_set_nz(byte); }
#define cpu_op_rol(byte) { cpu_data = cpu_p; if (byte & 0x80) cpu_p |= cpu_fc; else cpu_p &= ~cpu_fc; byte <<= 1; if (cpu_data & cpu_fc) byte |= 0x01; cpu_p_set_nz(byte); }
#define cpu_pull(byte) { cpu_s++; byte = cpu_addr[0x100 | cpu_s]; }
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
#define pha_op() { cpu_push(cpu_a); cpu_pw++; cpu_cl = 3; };
#define pla_op() { cpu_pull(cpu_a); cpu_pw++; cpu_cl = 4; };
#define php_op() { cpu_push(cpu_p); cpu_pw++; cpu_cl = 3; };
#define plp_op() { cpu_pull(cpu_p); cpu_pw++; cpu_cl = 4; };
#define tax_op() { cpu_x = cpu_a; cpu_p_set_nz(cpu_x); cpu_pw++; cpu_cl = 2; }
#define tay_op() { cpu_y = cpu_a; cpu_p_set_nz(cpu_y); cpu_pw++; cpu_cl = 2; }
#define tsx_op() { cpu_x = cpu_s; cpu_p_set_nz(cpu_x); cpu_pw++; cpu_cl = 2; }
#define txa_op() { cpu_a = cpu_x; cpu_p_set_nz(cpu_a); cpu_pw++; cpu_cl = 2; }
#define txs_op() { cpu_s = cpu_x;                      cpu_pw++; cpu_cl = 2; }
#define tya_op() { cpu_a = cpu_y; cpu_p_set_nz(cpu_a); cpu_pw++; cpu_cl = 2; }
// decrementors and incrementors
#define dec_zpg() { cpu_addr_load_zpg(); cpu_addr[cpu_bus]--; cpu_p_set_nz(cpu_addr[cpu_bus]); cpu_write++; cpu_cl = 5; }
#define dec_zpx() { cpu_addr_load_zpx(); cpu_addr[cpu_bus]--; cpu_p_set_nz(cpu_addr[cpu_bus]); cpu_write++; cpu_cl = 6; }
#define dex_op()  { cpu_x--; cpu_p_set_nz(cpu_x); cpu_pw++; cpu_cl = 2; }
#define dey_op()  { cpu_y--; cpu_p_set_nz(cpu_y); cpu_pw++; cpu_cl = 2; }
#define inc_zpg() { cpu_addr_load_zpg(); cpu_addr[cpu_bus]++; cpu_p_set_nz(cpu_addr[cpu_bus]); cpu_write++; cpu_cl = 5; }
#define inx_op()  { cpu_x++; cpu_p_set_nz(cpu_x); cpu_pw++; cpu_cl = 2; }
#define iny_op()  { cpu_y++; cpu_p_set_nz(cpu_y); cpu_pw++; cpu_cl = 2; }
#define asl_acc() { cpu_op_asl(cpu_a); cpu_pw++; cpu_cl = 2; }
#define lsr_acc() { cpu_op_lsr(cpu_a); cpu_pw++; cpu_cl = 2; }
#define rol_zpg() { cpu_addr_load_zpg(); cpu_op_rol(cpu_addr[cpu_bus]); cpu_write++; cpu_cl = 5; }
// maths
#define adc_imm() { cpu_pw++; cpu_op_adc(cpu_addr[cpu_pw]); cpu_pw++; cpu_cl = 2; }
#define adc_zpg() { cpu_addr_load_zpg(); cpu_op_adc(cpu_addr[cpu_bus]); cpu_cl = 3; }
#define sbc_imm() { cpu_pw++; cpu_op_sbc(cpu_addr[cpu_pw]); cpu_pw++; cpu_cl = 2; }
#define sbc_zpg() { cpu_addr_load_zpg(); cpu_op_sbc(cpu_addr[cpu_bus]); cpu_cl = 3; }
#define and_imm() { cpu_pw++; cpu_op_and(cpu_addr[cpu_pw]); cpu_pw++; cpu_cl = 2; }
#define and_zpg() { cpu_addr_load_zpg(); cpu_op_and(cpu_addr[cpu_bus]); cpu_cl = 3; }
#define eor_imm() { cpu_pw++; cpu_op_eor(cpu_addr[cpu_pw]); cpu_pw++; cpu_cl = 2; }
#define eor_zpg() { cpu_addr_load_zpg(); cpu_op_eor(cpu_addr[cpu_pw]); cpu_cl = 3; }
#define ora_imm() { cpu_pw++; cpu_op_ora(cpu_addr[cpu_pw]); cpu_pw++; cpu_cl = 2; }
#define ora_zpg() { cpu_addr_load_zpg(); cpu_op_ora(cpu_addr[cpu_bus]); cpu_cl = 3; }
#define ora_zpx() { cpu_addr_load_zpx(); cpu_op_ora(cpu_addr[cpu_bus]); cpu_cl = 4; }
#define ora_abs() { cpu_addr_load_abs(); cpu_op_ora(cpu_addr[cpu_bus]); cpu_cl = 4; }
#define ora_abx() { cpu_addr_load_abx(); cpu_op_ora(cpu_addr[cpu_bus]); cpu_cl = 5; }
#define ora_aby() { cpu_addr_load_aby(); cpu_op_ora(cpu_addr[cpu_bus]); cpu_cl = 5; }
#define ora_idx() { cpu_addr_load_idx(); cpu_op_ora(cpu_addr[cpu_bus]); cpu_cl = 6; }
#define ora_idy() { cpu_addr_load_idy(); cpu_op_ora(cpu_addr[cpu_bus]); cpu_cl = 6; }
// weirdos
#define bit_abs() { cpu_addr_load_abs(); cpu_op_bit(cpu_addr[cpu_bus]); cpu_read++; cpu_cl = 4; }
// branchers
#define bcc_op()  { cpu_branch(!(cpu_p & cpu_fc)); cpu_cl = 4; }
#define bcs_op()  { cpu_branch(  cpu_p & cpu_fc);  cpu_cl = 4; }
#define beq_op()  { cpu_branch(  cpu_p & cpu_fz);  cpu_cl = 4; }
#define bmi_op()  { cpu_branch(  cpu_p & cpu_fn);  cpu_cl = 4; }
#define bne_op()  { cpu_branch(!(cpu_p & cpu_fz)); cpu_cl = 4; }
#define bpl_op()  { cpu_branch(!(cpu_p & cpu_fn)); cpu_cl = 4; }
#define bvc_op()  { cpu_branch(!(cpu_p & cpu_fv)); cpu_cl = 4; }
#define bvs_op()  { cpu_branch(  cpu_p & cpu_fv);  cpu_cl = 4; }
#define jmp_abs() { cpu_addr_load_abs(); cpu_pw = cpu_bus; cpu_cl = 3; }
#define jmp_ind() { cpu_addr_load_ind(); cpu_pw = cpu_addr[cpu_bus]; cpu_pw += cpu_addr[cpu_bus + 1] << 8;  cpu_cl = 5; }
#define jsr_op()  { cpu_push((cpu_pw + 3) >> 8); cpu_push((cpu_pw + 3) & 0xff); cpu_addr_load_abs(); cpu_pw = cpu_bus; cpu_cl = 6; }
#define nmi_op()  { cpu_push((cpu_pw) >> 8); cpu_push((cpu_pw) & 0xff); cpu_pw = cpu_addr_nmi; cpu_push(cpu_p); cpu_cl = 6; }
#define rti_op()  { cpu_pull(cpu_p); cpu_pull(cpu_data); cpu_pw = cpu_data; cpu_pull(cpu_data); cpu_pw += (cpu_data << 8); cpu_cl = 6; }
#define rts_op()  { cpu_pull(cpu_data); cpu_pw = cpu_data; cpu_pull(cpu_data); cpu_pw += (cpu_data << 8); cpu_cl = 6; }
// comparers
#define cmp_imm() { cpu_pw++; cpu_p &= ~cpu_fc; if (cpu_a >= cpu_addr[cpu_pw]) cpu_p |= cpu_fc; cpu_p_set_nz((cpu_a - cpu_addr[cpu_pw]) & 0xff); cpu_pw++; cpu_cl = 2; }
#define cmp_zpg() { cpu_addr_load_zpg(); cpu_p &= ~cpu_fc; if (cpu_a >= cpu_addr[cpu_bus]) cpu_p |= cpu_fc; cpu_p_set_nz((cpu_a - cpu_addr[cpu_bus]) & 0xff); cpu_cl = 3; }
#define cmp_zpx() { cpu_addr_load_zpx(); cpu_p &= ~cpu_fc; if (cpu_a >= cpu_addr[cpu_bus]) cpu_p |= cpu_fc; cpu_p_set_nz((cpu_a - cpu_addr[cpu_bus]) & 0xff); cpu_cl = 4; }
#define cpx_imm() { cpu_pw++; cpu_p &= ~cpu_fc; if (cpu_x >= cpu_addr[cpu_pw]) cpu_p |= cpu_fc; cpu_p_set_nz((cpu_x - cpu_addr[cpu_pw]) & 0xff); cpu_pw++; cpu_cl = 2; }
#define cpy_imm() { cpu_pw++; cpu_p &= ~cpu_fc; if (cpu_y >= cpu_addr[cpu_pw]) cpu_p |= cpu_fc; cpu_p_set_nz((cpu_y - cpu_addr[cpu_pw]) & 0xff); cpu_pw++; cpu_cl = 2; }
#define cpy_zpg() { cpu_addr_load_zpg(); cpu_p &= ~cpu_fc; if (cpu_y >= cpu_addr[cpu_bus]) cpu_p |= cpu_fc; cpu_p_set_nz((cpu_y - cpu_addr[cpu_bus]) & 0xff); cpu_cl = 3; }
// loaders
#define lda_imm() { cpu_pw++; cpu_a = cpu_addr[cpu_pw]; cpu_p_set_nz(cpu_x); cpu_pw++; cpu_read++; cpu_cl = 2; }
#define lda_zpg() { cpu_addr_load_zpg(); cpu_a = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_a); cpu_read++; cpu_cl = 4; }
#define lda_zpx() { cpu_addr_load_zpx(); cpu_a = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_a); cpu_read++; cpu_cl = 4; }
#define lda_abs() { cpu_addr_load_abs(); cpu_a = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_a); cpu_read++; cpu_cl = 5; }
#define lda_abx() { cpu_addr_load_abx(); cpu_a = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_a); cpu_read++; cpu_cl = 5; }
#define lda_aby() { cpu_addr_load_aby(); cpu_a = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_a); cpu_read++; cpu_cl = 5; }
#define lda_idx() { cpu_addr_load_idx(); cpu_a = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_a); cpu_read++; cpu_cl = 6; }
#define lda_idy() { cpu_addr_load_idy(); cpu_a = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_a); cpu_read++; cpu_cl = 6; }
#define ldx_imm() { cpu_pw++; cpu_x = cpu_addr[cpu_pw]; cpu_p_set_nz(cpu_x); cpu_pw++; cpu_read++; cpu_cl = 2; }
#define ldx_zpg() { cpu_addr_load_zpg(); cpu_x = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_x); cpu_read++; cpu_cl = 4; }
#define ldx_aby() { cpu_addr_load_aby(); cpu_x = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_x); cpu_read++; cpu_cl = 5; }
#define ldy_imm() { cpu_pw++; cpu_y = cpu_addr[cpu_pw]; cpu_p_set_nz(cpu_y); cpu_pw++; cpu_read++; cpu_cl = 2; }
#define ldy_zpg() { cpu_addr_load_zpg(); cpu_y = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_y); cpu_read++; cpu_cl = 3; }
#define ldy_zpx() { cpu_addr_load_zpx(); cpu_y = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_y); cpu_read++; cpu_cl = 4; }
#define ldy_abs() { cpu_addr_load_abs(); cpu_y = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_y); cpu_read++; cpu_cl = 4; }
#define ldy_abx() { cpu_addr_load_abx(); cpu_y = cpu_addr[cpu_bus]; cpu_p_set_nz(cpu_y); cpu_read++; cpu_cl = 5; }
// storers
#define sta_abs() { cpu_addr_load_abs(); cpu_addr[cpu_bus] = cpu_a; cpu_write++; cpu_cl = 4; }
#define stx_abs() { cpu_addr_load_abs(); cpu_addr[cpu_bus] = cpu_x; cpu_write++; cpu_cl = 4; }
#define sty_abs() { cpu_addr_load_abs(); cpu_addr[cpu_bus] = cpu_y; cpu_write++; cpu_cl = 4; }
#define sta_abx() { cpu_addr_load_abx(); cpu_addr[cpu_bus] = cpu_a; cpu_write++; cpu_cl = 4; }
#define sta_aby() { cpu_addr_load_aby(); cpu_addr[cpu_bus] = cpu_a; cpu_write++; cpu_cl = 4; }
#define sta_zpg() { cpu_addr_load_zpg(); cpu_addr[cpu_bus] = cpu_a; cpu_write++; cpu_cl = 4; }
#define stx_zpg() { cpu_addr_load_zpg(); cpu_addr[cpu_bus] = cpu_x; cpu_write++; cpu_cl = 4; }
#define sty_zpg() { cpu_addr_load_zpg(); cpu_addr[cpu_bus] = cpu_y; cpu_write++; cpu_cl = 4; }
#define sta_zpx() { cpu_addr_load_zpx(); cpu_addr[cpu_bus] = cpu_a; cpu_write++; cpu_cl = 4; }

void cpu_crash(int opcode, int timeout) {
	if (opcode) {
		debug_out(1, "PROCESSOR DISCOMBOBULATION");
		debug_out(1, "2a03 undefined opcode: 0x%2X", opcode);
	}
	if (timeout) {
		debug_out(1, "TIMEOUT DEBUGGER");
	}
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
	debug_out(3, "PPU frames: %d", ppu_frame_count);
	debug_out(3, "CLK cycles: %d", mbu_cycle_count);
	debug_out(0, "NEFARIOUS CRASH EXIT");
	nes_running = 0;
}

void cpu_nmi() {
	nes_nmi = 0;
	nmi_op();
}

void cpu_irq() {
	nes_irq = 0;
}

void cpu_cycle() {
	if (cpu_cl) {
		cpu_cycle_count++;
		cpu_cl--;
		return;
	}
//	if (cpu_pw == 0xc068) debug_out(3, "NMI NMI NMI 0xZERO VALUE: 0x%2X", cpu_addr[0]);
//	if (cpu_pw == 0xc07F) debug_out(3, "OAMDMA WRITE  OAMDMA WRITE OAMDMA WRITE ");
//	if (cpu_pw == 0x0c07a) debug_out(3, "NMI JUMP TO STATE   %x", cpu_a);
//	if (cpu_pw == 0xc1d5) cpu_crash(0, 0);;
	if (cpu_cycle_count > 99999999) cpu_crash(0, cpu_cycle_count);
	uint8_t opcode = cpu_addr[cpu_pw];
	// DEBUG TOOLS & OPERATIONS
//	debug_out(3, "%4x %2x %2x %2x", cpu_pw, opcode, cpu_addr[cpu_pw+1], cpu_addr[cpu_pw+2]); // basic code crawler
//	debug_out(3, "A: %2x  X: %2x  Y: %2x  S: %2x", cpu_a, cpu_x, cpu_y, cpu_s);
//	debug_out(3, "%4x %2x %2x %2x   A: %2x  X: %2x  Y: %2x  S: %2x", cpu_pw, opcode, cpu_addr[cpu_pw+1], cpu_addr[cpu_pw+2], cpu_a, cpu_x, cpu_y, cpu_s);
/*
	debug_out(3, "C: %d  Z: %d  I: %d  D: %d  B: %d  V: %d  N: %d", 
		(cpu_p & cpu_fc) ? 1 : 0,
		(cpu_p & cpu_fz) ? 1 : 0,
		(cpu_p & cpu_fi) ? 1 : 0,
		(cpu_p & cpu_fd) ? 1 : 0,
		(cpu_p & cpu_fb) ? 1 : 0,
		(cpu_p & cpu_fv) ? 1 : 0,
		(cpu_p & cpu_fn) ? 1 : 0
	);
	*/
//	debug_out(3, "stack: %2x %2x %2x %2x %2x %2x %2x %2x", cpu_addr[0x1fa], cpu_addr[0x1fb], cpu_addr[0x1fc], cpu_addr[0x1fb], cpu_addr[0x1fc], cpu_addr[0x1fd], cpu_addr[0x1fe], cpu_addr[0x1ff]); // top 8 bytes of stack
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
		case 0x48: pha_op(); break;
		case 0x68: pla_op(); break;
		case 0x08: php_op(); break;
		case 0x28: plp_op(); break;
		case 0xaa: tax_op(); break;
		case 0xa8: tay_op(); break;
		case 0xba: tsx_op(); break;
		case 0x8a: txa_op(); break;
		case 0x9a: txs_op(); break;
		case 0x98: tya_op(); break;
		// decrementors and incrementors
		case 0xc6: dec_zpg(); break;
		case 0xd6: dec_zpx(); break;
		case 0xca: dex_op();  break;
		case 0x88: dey_op();  break;
		case 0xe6: inc_zpg(); break;
		case 0xe8: inx_op();  break;
		case 0xc8: iny_op();  break;
		case 0x0a: asl_acc(); break;
		case 0x4a: lsr_acc(); break;
		case 0x26: rol_zpg(); break;
		// maths
		case 0x69: adc_imm(); break;
		case 0x65: adc_zpg(); break;
		case 0xe9: sbc_imm(); break;
		case 0xe5: sbc_zpg(); break;
		case 0x29: and_imm(); break;
		case 0x25: and_zpg(); break;
		case 0x49: eor_imm(); break;
		case 0x45: eor_zpg(); break;
		case 0x09: ora_imm(); break;
		case 0x05: ora_zpg(); break;
		case 0x15: ora_zpx(); break;
		case 0x0d: ora_abs(); break;
		case 0x1d: ora_abx(); break;
		case 0x19: ora_aby(); break;
		case 0x01: ora_idx(); break;
		case 0x11: ora_idy(); break;
		// weirdos
		case 0x2c: bit_abs(); break;
		// branchers
		case 0x90: bcc_op();  break;
		case 0xb0: bcs_op();  break;
		case 0xf0: beq_op();  break;
		case 0x30: bmi_op();  break;
		case 0xd0: bne_op();  break;
		case 0x10: bpl_op();  break;
		case 0x50: bvc_op();  break;
		case 0x70: bvs_op();  break;
		case 0x4c: jmp_abs(); break;
		case 0x6c: jmp_ind(); break;
		case 0x20: jsr_op();  break;
		case 0x40: rti_op();  break;
		case 0x60: rts_op();  break;
		// comparers
		case 0xc9: cmp_imm(); break;
		case 0xc5: cmp_zpg(); break;
		case 0xd5: cmp_zpx(); break;
		case 0xe0: cpx_imm(); break;
		case 0xc0: cpy_imm(); break;
		case 0xc4: cpy_zpg(); break;
		// loaders
		case 0xa9: lda_imm(); break;
		case 0xa5: lda_zpg(); break;
		case 0xb5: lda_zpx(); break;
		case 0xad: lda_abs(); break;
		case 0xbd: lda_abx(); break;
		case 0xb9: lda_aby(); break;
		case 0xa1: lda_idx(); break;
		case 0xb1: lda_idy(); break;
		case 0xa2: ldx_imm(); break;
		case 0xa6: ldx_zpg(); break;
		case 0xbe: ldx_aby(); break;
		case 0xa0: ldy_imm(); break;
		case 0xa4: ldy_zpg(); break;
		case 0xb4: ldy_zpx(); break;
		case 0xac: ldy_abs(); break;
		case 0xbc: ldy_abx(); break;
		// storers
		case 0x8d: sta_abs(); break;
		case 0x8e: stx_abs(); break;
		case 0x8c: sty_abs(); break;
		case 0x9d: sta_abx(); break;
		case 0x99: sta_aby(); break;
		case 0x85: sta_zpg(); break;
		case 0x86: stx_zpg(); break;
		case 0x84: sty_zpg(); break;
		case 0x95: sta_zpx(); break;

		default: cpu_crash(opcode, 0); break;
	}
}
