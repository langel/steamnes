/*
	Developer's Pipeline

	watches a source file
	file modification shuts down program
	a batch file looper can recompile and launch
*/

int dpipe_watch_time;

// XXX this shouldn't be hard coded
char * dpipe_watch_file = "main.c";

int dpipe_get_watch_time() {
	struct stat buff;
	stat(dpipe_watch_file, &buff);
	return (int) buff.st_mtime;
}

void dpipe_init(SDL_Window * window) {
	dpipe_watch_time = dpipe_get_watch_time();
	window_focus(window);
}

int dpipe_check_update() {
	// returns true if watch file has been updated
	return (dpipe_watch_time == dpipe_get_watch_time()) ? 0 : 1;
}

void dpipe_kill_cycle() {
	FILE * fp = fopen("dpipe_death", "w");
	fclose(fp);
}

