/*
	Random Number Generator eXPerience
*/


// SQUIRREL3 predictive noise
// discovered here : https://www.youtube.com/watch?v=LWFzPP8ZbdU

uint32_t squirrel3(uint32_t position, uint32_t seed) {
	const uint32_t BIT_NOISE1 = 0xB5297A4D;
	const uint32_t BIT_NOISE2 = 0x68E31DA4;
	const uint32_t BIT_NOISE3 = 0x1B56C4E9;

	uint32_t mangled = position;
	mangled *= BIT_NOISE1;
	mangled += seed;
	mangled ^= (mangled >> 8);
	mangled += BIT_NOISE2;
	mangled ^= (mangled << 8);
	mangled *= BIT_NOISE3;
	mangled ^= (mangled >> 8);
	return mangled;
}

uint32_t squirrel3_get_2d(int x, int y, uint32_t seed) {
	const int prime = 198491317;
	return squirrel3((uint32_t) (x + y * prime), seed);
}

uint32_t squirrel3_get_3d(int x, int y, int z, uint32_t seed) {
	const int prime1 = 198491317;
	const int prime2 = 6542989;
	return squirrel3((uint32_t) (x + y * prime1 + z * prime2), seed);
}

// returns float between 0 and 1
float squirrel3_zero_float(uint32_t position, uint32_t seed) {
	return (float) squirrel3(position, seed) / (float) 0xffffffff;
}

// returns float between -1 and 1
float squirrel3_neg_float(uint32_t position, uint32_t seed) {
	return ((float) squirrel3(position, seed) / (float) 0x7fffffff) - 1.f;
}


// Linear Feedback Shift Registers
// note very tested

int rng_lfsr_8bit_next(uint8_t *val, int max) {
	*val ^= (*val & 0x07) << 5;
	*val ^= *val >> 3;
	*val ^= (*val & 0x03) << 6;
	return (int)( ( (float) *val / (float) 0xff) * (float) max );
}

int rng8(int max) {
	static uint8_t val = 1;
	return rng_lfsr_8bit_next(&val, max);
}

int rng_lfsr_16bit_next(uint16_t *val, int max) {
	*val ^= (*val & 0x07ff) << 5;
	*val ^= *val >> 7;
	*val ^= (*val & 0x0003) << 14;
	return (int)( ( (float) *val / (float) 0xffff) * (float) max );
}

int rng16(int max) {
	static uint16_t val = 1;
	return rng_lfsr_16bit_next(&val, max);
}

int rng_lfsr_32bit_next(uint32_t *val, int max) {
	*val ^= (*val & 0x0007ffff) << 13;
	*val ^= *val >> 17;
	*val ^= (*val & 0x07ffffff) << 5;
	return (int)( ( (float) *val / (float) 0xffffffff) * (float) max );
}

int rng32(int max) {
	static uint32_t val = 1;
	return rng_lfsr_32bit_next(&val, max);
}

/*
	32 bit rng lfsr does not seem to be working
	too many repeats in rng_test

	solution found here:
	https://stackoverflow.com/questions/65661856/how-to-do-an-8-bit-16-bit-and-32-bit-linear-feedback-shift-register-prng-in-ja

	should look more into Xorshift:
	https://en.wikipedia.org/wiki/Xorshift
*/


int rng32alt() {
	// stolen from: https://www.excamera.com/sphinx/article-xorshift.html
	/* if this proves to be superior we should add a custom seed option */
	static uint32_t seed = 7;
	seed ^= seed << 13;
	seed ^= seed >> 17;
	seed ^= seed << 5;
	return seed;
}

/*
	Noise Functions aka Hashes
	ideas from gdc video: https://www.youtube.com/watch?v=LWFzPP8ZbdU
	instead of sequencing pseudo random numbers apply hashes to linear ints
	python version of his hash (shouldn't be hard to make into c): https://github.com/sublee/squirrel3-python/blob/master/squirrel3.py
	he also recomended using std::hash
*/


// Perlin Noise
// this doesn't seem to work appropriately either

int perlin_series[512] = {
	151, 160, 137,  91,  90,  15, 131,  13, 201,  95,  96,  53, 194, 233,   7, 225,
	140,  36, 103,  30,  69, 142,   8,  99,  37, 240,  21,  10,  23, 190,   6, 148,
	247, 120, 234,  75,   0,  26, 197,  62,  94, 252, 219, 203, 117,  35,  11,  32,
	 57, 177,  33,  88, 237, 149,  56,  87, 174,  20, 125, 136, 171, 168,  68, 175,
	 74, 165,  71, 134, 139,  48,  27, 166,  77, 146, 158, 231,  83, 111, 229, 122,
	 60, 211, 133, 230, 220, 105,  92,  41,  55,  46, 245,  40, 244, 102, 143,  54,
	 65,  25,  63, 161,   1, 216,  80,  73, 209,  76, 132, 187, 208,  89,  18, 169,
	200, 196, 135, 130, 116, 188, 159,  86, 164, 100, 109, 198, 173, 186,   3,  64,
	 52, 217, 226, 250, 124, 123,   5, 202,  38, 147, 118, 126, 255,  82,  85, 212,
	207, 206,  59, 227,  47,  16,  58,  17, 182, 189,  28,  42, 223, 183, 170, 213,
	119, 248, 152,   2,  44, 154, 163,  70, 221, 153, 101, 155, 167,  43, 172,   9,
	129,  22,  39, 253,  19,  98, 108, 110,  79, 113, 224, 232, 178, 185, 112, 104,
	218, 246,  97, 228, 251,  34, 242, 193, 238, 210, 144,  12, 191, 179, 162, 241,
	 81,  51, 145, 235, 249,  14, 239, 107,  49, 192, 214,  31, 181, 199, 106, 157,
	184,  84, 204, 176, 115, 121,  50,  45, 127,   4, 150, 254, 138, 236, 205,  93,
	222, 114,  67,  29,  24,  72, 243, 141, 128, 195,  78,  66, 215,  61, 156, 180
};

double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }

// XXX defined elsewhere using float
//double lerp(double t, double a, double b) { return a + t * (b - a); }

double grad(int hash, double x, double y, double z) {
	int h = hash & 15;
	double u = (h < 8) ? x : y;
	double v = (h < 4) ? y : (h == 12 || h == 14) ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double noise(double x, double y, double z) {
	int X = (int)floor(x) & 255;
	int Y = (int)floor(y) & 255;
	int Z = (int)floor(z) & 255;
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);
	int A = perlin_series[X  ]+Y, AA = perlin_series[A]+Z, AB = perlin_series[A+1]+Z,
		 B = perlin_series[X+1]+Y, BA = perlin_series[B]+Z, BB = perlin_series[B+1]+Z;

	return lerp(w, lerp(v, lerp(u, grad(perlin_series[AA  ], x  , y  , z   ),
											 grad(perlin_series[BA  ], x-1, y  , z   )),
								  lerp(u, grad(perlin_series[AB  ], x  , y-1, z   ),
											 grad(perlin_series[BB  ], x-1, y-1, z   ))),
						lerp(v, lerp(u, grad(perlin_series[AA+1], x  , y  , z-1 ),
											 grad(perlin_series[BA+1], x-1, y  , z-1 )),
								  lerp(u, grad(perlin_series[AB+1], x  , y-1, z-1 ),
											 grad(perlin_series[BB+1], x-1, y-1, z-1 ))));
}
