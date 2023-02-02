#define debug_str_maxlen 2048

enum debug_output_target_types {
	none,
	cli,
	gui,
	all,
};
enum debug_output_target_types debug_output_target;

int debug_level = 0;
char debug_str[debug_str_maxlen];

void debug_out(int level, char* str, ...) {
	// prep string
	va_list args;
	va_start(args, str);
	vsprintf(debug_str, str, args);
	va_end(args);
	// make sure str is nulled out with room for newline
	for (int i = 0; i <= 3; i++) {
		debug_str[debug_str_maxlen - i] = 0x0;
	}
	// concatenate newline
	strcat(debug_str, "\n");
	// check for appropriate severity
	if (level <= debug_level) {
		// command line interface
		if (debug_output_target == cli || debug_output_target == all) {
			printf(debug_str);
		}
		// graphical user interface
		if (debug_output_target == gui || debug_output_target == all) {
			// XXX we need a gui
		}
	}
}
