#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#define NAME_MAX 255  // max name for a file
#define BUFFER_SIZE sizeof(struct inotify_event) + NAME_MAX + 1

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Please provide a file to track \n");

		exit(EXIT_FAILURE);
	}

	char *filePath = argv[1];
	char buffer[4090];
	ssize_t length;

	const struct inotify_event *events;

	printf("The given file is: %s\n\n", filePath);

	// check if the file exists
	if (access(filePath, F_OK) == -1) {
		fprintf(stderr, "The provided file: %s, does not exits\n ", filePath);
		exit(EXIT_FAILURE);
	}

	// init inotify
	int fileDescriptor = inotify_init();

	if (fileDescriptor == -1) {
		perror("Unable to initialize inotify instance");
		exit(EXIT_FAILURE);
	}
	printf("Successfully initiated inotify object\n");

	// add file to inotify watch list and listen for all events
	int watchFile = inotify_add_watch(fileDescriptor, filePath, IN_ALL_EVENTS);

	if (watchFile == -1) {
		perror("Unable to add file to the watch list");
		exit(EXIT_FAILURE);
	}

	printf("-------Daemon has started--------\n");

	// main daemon loop
	while (1) {

		length = read(fileDescriptor, buffer, sizeof(buffer));

		if (length <= 0) {
			perror("Error reading the file descriptor");
			break;	// TODO: why break
		}

		for (char *bufferPtr = buffer; bufferPtr < buffer + length;
			 bufferPtr += sizeof(struct inotify_event) + events->len) {

			events = (const struct inotify_event *)bufferPtr;

			if (events->mask & IN_ACCESS)
				printf("The File has been accessed\n");

			if (events->mask & IN_DELETE)
				printf("The File has been Deleted\n");

			if (events->mask & IN_MODIFY)
				printf("The File has been Modified \n");

			if (events->mask & IN_CLOSE_WRITE)
				printf("The file was written to and closed \n");
		}
	}

	// i += sizeof(struct inotify_event) + events->len;

	if (close(fileDescriptor)) {
		printf("error closing the file descriptor\n");
	}
	return 0;
}
