/*
	Audio Processing Unit
	apu.c
*/

uint8_t apu_regs[18];
int apu_frame_counter = 0;
float apu_samples_per_cycle = 0.f;
float apu_samples_pos = 0.f;
float apu_pinout = 0.f;
float apu_out = 0.f;
float apu_out_prev = 0.f;

float apu_hpf_cutoff = 0.f;
float apu_hpf_input = 0.f;
float apu_hpf_output = 0.f;
float apu_lpf_cutoff = 0.f;
float apu_lpf_input = 0.f;
float apu_lpf_output = 0.f;

int apu_tri_length_counter = 0;
int apu_tri_length_halt = 0;
int apu_tri_linear_counter = 0;
int apu_tri_linear_reload = 0;
int apu_tri_timer_counter = 0;
int apu_tri_seq_pos = 0;
float apu_tri_sample = 0.f;
float apu_tri_freq = 0.f;

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

const int apu_length_table[32] = {
	10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14,
	12, 26, 14, 12, 16, 24, 18, 48, 20, 96, 22, 192, 
	24, 72, 26, 16, 28, 32, 30,
};
 

void apu_clock() {
	// half frame interrupts
	if (apu_frame_counter == 3728 || apu_frame_counter == 11185) {
		if (apu_tri_linear_counter > 0) apu_tri_linear_counter--;
		// pulse envelopes
	}
	// quarter frame interrupts
	if (apu_frame_counter == 7456 || apu_frame_counter == 14914) {
		if (apu_tri_linear_counter > 0) apu_tri_linear_counter--;
		// pulse envelopes
		if (apu_tri_length_halt) {
			apu_tri_linear_counter = cpu_addr[0x4008] & 0x7f;
		}
		else {
			if (apu_tri_length_counter > 0) apu_tri_length_counter--;
			apu_tri_linear_reload = 0;
		}
		// pulse sweeps
	}
	// reset frame interrupts
	if (apu_frame_counter == 14914) {
		apu_frame_counter = 0;
	}
	apu_frame_counter++;
	if (--apu_tri_timer_counter <= 0) {
		apu_tri_seq_pos = (apu_tri_seq_pos + 1) & 0x1f;
		apu_tri_timer_counter = (int) ((cpu_addr[0x400a] | ((cpu_addr[0x400b] & 0x7) << 8)) + 1);
		apu_tri_sample = (float) apu_tri_sequence[apu_tri_seq_pos] - 7.5f;
		apu_tri_freq = (float) clock_ntsc / 32.f / (float) apu_tri_timer_counter;
		//debug_out(3, "%X %X", apu_tri_sample, apu_tri_seq_pos);
		//debug_out(3, "%b", apu_tri_timer_counter);
	}
	if (!((cpu_addr[0x4015] & 0x4) && apu_tri_length_counter && apu_tri_linear_counter)) {
		apu_tri_sample = 0.f;
	}
	apu_pinout = apu_tri_sample * 0.1f;
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
		// depopper
		const float depopper = 0.075f;
		apu_out = apu_out_prev + depopper * (apu_pinout - apu_out_prev);
		apu_out_prev = apu_out;
		// hi pass filter
		float filter_out = apu_hpf_cutoff * (apu_hpf_output + apu_out - apu_hpf_input);
		apu_hpf_input = apu_out;
		apu_hpf_output = filter_out;
		// lo pass filter
		filter_out = apu_lpf_output + apu_lpf_cutoff * (apu_hpf_output - apu_lpf_output);
		apu_lpf_output = filter_out;
		// channle output
		output_l = output_r = filter_out;
		//output_l = sinf((float) frame_counter * 0.01f);
		float_stream[i] = output_l;
		float_stream[i+1] = output_r;
	}
}


void apu_init() {
	debug_out(3, "initializing apu");
	audio_init(48000, 2, 512, AUDIO_F32SYS, &apu_buffer_out);
	float cpu_hz = (float) clock_ntsc / 12.f;
	apu_samples_per_cycle = (float) audio_spec.freq / cpu_hz;
	debug_out(3, "samples per cycle: %f", apu_samples_per_cycle);
	// init filters (resistor-capacitor / delta-time)
	float cutoff_freq, rc, dt;
	dt = 1.0f / audio_spec.freq;
	// init hi pass filter
	cutoff_freq = 20.f;
	rc = 1.0f / (2.0f * (float) M_PI * cutoff_freq);
	apu_hpf_cutoff = rc / (rc + dt);
	// init lo pass filter
	cutoff_freq = 14000.f;
	rc = 1.0f / (2.0f * (float) M_PI * cutoff_freq);
	apu_lpf_cutoff = dt / (rc + dt);
}

