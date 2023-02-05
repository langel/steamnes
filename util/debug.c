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
	// check for appropriate severity
	if (level <= debug_level) {
		// prep string
		va_list args;
		va_start(args, str);
		vsprintf(debug_str, str, args);
		va_end(args);
		// command line interface
		if (debug_output_target == cli || debug_output_target == all) {
			printf("%s\n", debug_str);
		}
		// graphical user interface
		if (debug_output_target == gui || debug_output_target == all) {
			// XXX we need a gui
		}
	}
}
