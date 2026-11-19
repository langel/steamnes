/*
	Audio Processing Unit
	apu.c
*/

uint8_t apu_regs[18];
float apu_samples_per_cycle = 0.f;
float apu_samples_pos = 0.f;
float apu_pinout = 0.f;

int apu_tri_length_counter = 0;
int apu_tri_seq_pos = 0;
int apu_tri_sample = 0;

const uint16_t apu_pulse_duty_0[4] = {
	0b0010000000100000,
	0b0011000000110000,
	0b0011110000111100,
	0b1100111111001111,
};

const int apu_tri_sequence[32] = {
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
};
 
void apu_clock() {
	apu_tri_length_counter--;
	if (apu_tri_length_counter < 0) {
		apu_tri_seq_pos++;
		apu_tri_seq_pos &= 0x1f;
		apu_tri_length_counter = cpu_addr[0x400a] + (cpu_addr[0x400b] << 8);
		apu_tri_sample = apu_tri_sequence[apu_tri_seq_pos];
		//debug_out(3, "%X %X", apu_tri_sample, apu_tri_seq_pos);
		//debug_out(3, "%b", apu_tri_length_counter);
	}
	apu_pinout = (float) apu_tri_sample * 0.1f;
}

void apu_buffer_out(void* userdata, uint8_t* byte_stream, int byte_stream_length) {
	
	float * float_stream = (float*) byte_stream;
	int float_stream_length = byte_stream_length >> 2;
	
	for (int i = 0; i < float_stream_length; i += 2) {
		float output_l = 0.f;
		float output_r = 0.f;
		while (apu_samples_pos <= 1.f) {
			apu_clock();
			apu_samples_pos += apu_samples_per_cycle;
		}
		apu_samples_pos -= 1.f;
		output_l = output_r = apu_pinout;
		//output_l = sinf((float) frame_counter * 0.01f);
		float_stream[i] = output_l;
		float_stream[i+1] = output_r;
	}

}

void apu_init() {
	debug_out(3, "initializing apu");
	audio_init(48000, 2, 1024, AUDIO_F32SYS, &apu_buffer_out);
	apu_samples_per_cycle = (float) audio_spec.freq / (float) clock_ntsc;
	debug_out(3, "samples per cycle: %f", apu_samples_per_cycle);
}

