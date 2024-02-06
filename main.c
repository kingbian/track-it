#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

int main(int argc, char* argv[]) {

	if (argc < 2) {
		printf("Please provide a file to track \n");
		return 1;
	}

	// char** fileName = argv + 1;
	// for (int i = 0; i < argc - 1; ++i) {
	// 	printf("File[%d]: %s\n", i, fileName[i]);
	// }

	// printf("Read into array");
	// for (int i = 1; i < argc; ++i) {
	// 	printf("File[%d]: %s\n", i, fileName[i]);
	// }

	// check if the file exists

	if (access(argv[1], F_OK) == -1) {
		fprintf(stderr, "The provided file: %s, does not exits\n ", argv[1]);
		exit(EXIT_FAILURE);
	}

	// int daemonize = daemon(1, 1);

	// if (daemonize != 0) {
	// 	printf("Unable to daemonize process\n");
	// 	return -1;
	// }

	printf("-------Daemon has started--------\n");

	return 0;
}