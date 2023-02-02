uint8_t* rom_header;
char rom_header_type_str[16];

enum rom_file_types {
	unknown,
	iNES,
	NES2,
};
enum rom_file_types rom_file_type;

char* rom_filename;
int rom_has_trainer;
int rom_prg_rom_size;
int rom_chr_rom_size;
int rom_chr_rom_start;

int rom_load(char* filename) {
	rom_filename = malloc(strlen(filename));
	strcpy(rom_filename, filename);
	FILE* rom = fopen(filename, "rb");
	if (rom == NULL) {
		printf("failed access to : %s\n", filename);
		return 0;
	}
	rom_header = malloc(16);
	fread(rom_header, 16, 1, rom);
	// zero out rom properties 
	rom_file_type = unknown;
	rom_has_trainer = 0;
	rom_prg_rom_size = 0;
	rom_chr_rom_size = 0;
	rom_chr_rom_start = 0;
	strcpy(rom_header_type_str, "NO HEADER");
	if (rom_header[0]=='N' 
	&& rom_header[1]=='E' 
	&& rom_header[2]=='S' 
	&& rom_header[3]==0x1A) {
		rom_file_type = iNES;
		strcpy(rom_header_type_str, "iNES");
		// byte 4 PRG ROM data (16384 * x bytes)
		rom_prg_rom_size = rom_header[4] << 14;
		// byte 5 CHR ROM data, if present (8192 * y bytes)
		rom_chr_rom_size = rom_header[5] << 13;
		// byte 6 has misc but trainer important
		if (rom_header[6] & 0x04) {
			rom_has_trainer = 512;
		}
		// byte 7 has misc and also NES2 flag
		if ((rom_header[7] & 0x0C) == 0x08) {
			rom_file_type = NES2;
			strcpy(rom_header_type_str, "NES2.0");
		}
		rom_chr_rom_start = 16 + rom_has_trainer;
		if (rom_chr_rom_size) {
			rom_chr_rom_start += rom_prg_rom_size;
		}
		else {
			rom_chr_rom_size = rom_prg_rom_size;
		}
	}
	else {
		fseek(rom, 0, SEEK_END);
		rom_chr_rom_size = ftell(rom);
	}
	fclose(rom);
	free(rom_header);
	return 1;
}
