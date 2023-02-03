
enum rom_header_types {
	unknown,
	iNES,
	NES2,
};
enum rom_header_types rom_header_type;

char* rom_filename;
uint8_t* rom_header;
char rom_header_type_str[16];
int rom_mapper;
int rom_has_trainer;
int rom_prg_size;
uint8_t* rom_prg_data;
int rom_chr_size;
uint8_t* rom_chr_data;
int rom_chr_start;


int rom_load(char* filename) {
	// XXX TODO
	// chr and prg ram
	// NES2.0 higher mapper nybble
	rom_filename = malloc(strlen(filename));
	strcpy(rom_filename, filename);
	FILE* rom = fopen(filename, "rb");
	if (rom == NULL) {
		debug_out(1, "failed to access %s", filename);
		return 1;
	}
	rom_header = malloc(16);
	fread(rom_header, 16, 1, rom);
	// zero out rom properties 
	rom_header_type = unknown;
	rom_mapper = 0;
	rom_has_trainer = 0;
	rom_prg_size = 0;
	rom_chr_size = 0;
	rom_chr_start = 0;
	strcpy(rom_header_type_str, "NO HEADER");
	char header_check[5] = {'N', 'E', 'S', 0x1A, 0x0};
	for (int i = 0; i < 4; i++) {
		if (rom_header[i] != header_check[i]) {
			debug_out(1, "failed to identify NES header");
			return 1;
		}
	}
	rom_header_type = iNES;
	strcpy(rom_header_type_str, "iNES");
	// byte 4 PRG ROM data (16384 * x bytes)
	rom_prg_size = rom_header[4] << 14;
	// byte 5 CHR ROM data, if present (8192 * y bytes)
	rom_chr_size = rom_header[5] << 13;
	// byte 6 has mapper, trainer, misc.
	rom_mapper = rom_header[6] >> 4;
	if (rom_header[6] & 0x04) {
		rom_has_trainer = 512;
	}
	// byte 7 has mapper, NES2 flag, misc.
	rom_mapper += (rom_header[7] & 0xf0);
	if ((rom_header[7] & 0x0C) == 0x08) {
		rom_header_type = NES2;
		strcpy(rom_header_type_str, "NES2.0");
	}
	if (rom_chr_size) {
		// compensate for header size
		rom_chr_start = 16 + rom_has_trainer;
		// compensate for program size
		rom_chr_start += rom_prg_size;
	}
	// debug status
	debug_out(3, "%s header found", rom_header_type_str);
	debug_out(3, "PRG ROM data size %i", rom_prg_size);
	debug_out(3, "CHR ROM data size %i", rom_chr_size);
	debug_out(3, "Mapper %i", rom_mapper);
	// shut it down
	fclose(rom);
	free(rom_header);
	return 0;
}
