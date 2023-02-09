// XXX not Windows compatible :(
//#include <execinfo.h>

#define debug_trace_maxlen 256
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

void debug_trace() {
	/*
	void* array[debug_trace_maxlen];
	char** strings;
	int size;
	size = backtrace(array, debug_trace_maxlen);
	strings = backtrace_symbols(array, debug_trace_maxlen);
	if (strings != NULL) {
		printf("debug_trace found %d pointers:\n", size - 1);
		for (int i = 1; i < size; i++) {
			printf("   %s\n", strings[i]);
		}
	}
	free(strings);
	*/
}

void debug_out(int level, char* str, ...) {
	// check for appropriate severity
	if (level <= debug_level) {
		// check string length
		if (strlen(str) > debug_str_maxlen) {
			printf("debug_out string exceeds %d character max length\n  \"%s\"\n", debug_str_maxlen, str);
			debug_trace();
		}
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

