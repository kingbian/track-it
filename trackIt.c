#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#define NAME_MAX 255  // max name for a file
#define BUFFER_SIZE sizeof(struct inotify_event) + NAME_MAX + 1

void sendNotification(char *message, char *fileName, char *event);
void daemonize(int fd, int watchFile, char *filePath);

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Please provide a file to track \n");

		exit(EXIT_FAILURE);
	}

	char *filePath = argv[1];
	ssize_t length;

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

	// add file to inotify watch list and listen for all events
	int watchFile = inotify_add_watch(fileDescriptor, filePath, IN_ALL_EVENTS);

	if (watchFile == -1) {
		perror("Unable to add file to the watch list");
		exit(EXIT_FAILURE);
	}

	printf("-------Daemon has started--------\n");

	daemonize(fileDescriptor, watchFile, filePath);

	if (close(fileDescriptor)) {
		printf("error closing the file descriptor\n");
	}
	return 0;
}

/**
 * function to daemonize the program
 *
 */

void daemonize(int fd, int watchFile, char *filePath) {

	char *message, *file, *event;
	ssize_t length = -1;

	struct inotify_event *events;
	char buffer[4090];
	message = file = event = NULL;

	// main daemon loop
	while (1) {

		length = read(fd, buffer, sizeof(buffer));

		if (length <= 0) {
			perror("Error reading the file descriptor");
			break;
		}

		char *eventPtr = buffer;

		message = NULL;
		// iterate over the events
		while (eventPtr < buffer + length) {

			events = (struct inotify_event *)eventPtr;

			if (events->mask & IN_ACCESS || events->mask & IN_OPEN) {

				event = "File Access";
				message = "The File has been accessed\n";
			}

			if (events->mask & IN_DELETE) {

				event = "File Deleted: ";
				message = "The File has been Deleted\n";
			}

			if (events->mask & IN_MODIFY) {

				event = "File was modified";
				message = "The File has been Modified \n";
			}

			if (events->mask & IN_CLOSE_WRITE) {
				event = "File  write/close";
				message = "The file was written to and closed \n";
			}

			// increment eventPtr position in the buffer
			long eventSize = sizeof(struct inotify_event) + events->len;
			eventPtr += eventSize;
		}

		if (message) {
			sendNotification(message, filePath, event);
		}
	}
}

void sendNotification(char *message, char *fileName, char *event) {

	// format the notification title
	char notificationTitle[NAME_MAX + 100];
	snprintf(notificationTitle, sizeof(notificationTitle), "%s: %s", fileName, event);
	char command[NAME_MAX + 100];

	sprintf(command, "notify-send '%s' '%s'", notificationTitle, message);
	system(command);
}