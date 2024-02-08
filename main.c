#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#define BUFFER_SIZE (10 * (sizeof(struct inotify_event) + 255 + 1))

int main(int argc, char* argv[]) {

	// if (argc < 2) {
	// 	printf("Please provide a file to track \n");

	// 	exit(EXIT_FAILURE);
	// }

	FILE* tracker = fopen("track.txt", "w");
	char* filePath = "/home/heisenberg/projects/track-it/test.txt";
	char buffer[BUFFER_SIZE];

	printf("Writing to the file %s\n", tracker);
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

	printf("Successfully added file: %s to inotify watch list\n", filePath);

	printf("-------Daemon has started--------\n");

	// main daemon loop
	while (1) {

		int length = read(watchFile, buffer, sizeof(buffer));

		if (length < 0) {
			perror("Error reading the file descriptor");
			exit(EXIT_FAILURE);
		}

		// // check if an event has occurred

		// if (length == 0) continue;	// nothing has happened

		for (int i = 0; i < length; ++i) {

			struct inotify_event* events = (struct inotify_event*)&buffer[i];

			switch (events->mask) {
				case IN_ACCESS:
					fprintf(tracker, "The File has been accessed\n");
					printf("The File has been accessed\n");
					break;
				case IN_ATTRIB:
					fprintf(tracker, "The File has been changed\n");
					printf("The File's meta data has been changed\n");
					break;

				case IN_DELETE:
					printf("The File has been Deleted\n");
					break;
				case IN_OPEN:
					printf("The File has been Opened \n");
					break;
				case IN_MODIFY:
					printf("The File has been Modified \n");
					break;
				default:
					break;
			}
			i += sizeof(struct inotify_event) + events->len;
		}
	}

	fclose(tracker);
	return 0;
}