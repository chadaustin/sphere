#include "unix_time.h"
#include "../sphere.hpp"
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include "unix_audio.h"

#ifndef DATADIR
#define DATADIR "."
#endif

// should this just be FILENAME_MAX?
#define MAX_UNIX_PATH 256

#define stringize(s) (#s)
static char unix_data_dir[MAX_UNIX_PATH] = stringize(DATADIR);
static char* original_directory;

int main(int argc, const char* argv[]) {
	original_directory = getcwd(NULL, 0);
	char* env_data_dir = getenv("SPHERE_DATA_DIR");
	if (env_data_dir != NULL)
		strcpy(unix_data_dir, env_data_dir);
	if (getopt(argc, const_cast<char**>(argv), "d:") == 'd') {
		strcpy(unix_data_dir, optarg);
	}
	chdir(unix_data_dir);
	srand((unsigned)GetTime);
	if (!InitAudio()) {
		printf("Sound could not be initialized...\n");
	}
	RunSphere(argc, argv);
	CloseAudio();
}
