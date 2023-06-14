/*
	Developer's Pipeline

	watches a source file
	file modification shuts down program
	a batch file looper can recompile and launch
*/

int dpipe_watch_time;
int dpipe_enabled = 0;
int dpipe_kill = 0;

// XXX this shouldn't be hard coded
char dpipe_watch_file[256];

int dpipe_get_watch_time() {
	struct stat buff;
	stat(dpipe_watch_file, &buff);
	return (int) buff.st_mtime;
}

void dpipe_init(char * const watch_file) {
	strcpy(dpipe_watch_file, watch_file);
	dpipe_watch_time = dpipe_get_watch_time();
	dpipe_enabled++;
}

void dpipe_update() {
	// returns true if watch file has been updated
	if (!dpipe_enabled) return;
	if (dpipe_watch_time == dpipe_get_watch_time()) return;
	fcl_running = 0;
}

void dpipe_quit() {
	printf("%d %d\n", dpipe_enabled, dpipe_kill);
	if (!dpipe_enabled || !dpipe_kill) return;
	FILE * fp = fopen("dpipe_death", "w");
	fclose(fp);
}

