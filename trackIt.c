#include <fcntl.h>
#include <libnotify/notify.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

#define NAME_MAX 255  // max name for a file
#define BUFFER_SIZE sizeof(struct inotify_event) + NAME_MAX + 1

void sendNotification(char *message, char *fileName, char *event);
void daemonize(int fd, char **watchFiles);

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Please provide a file to track \n");

		exit(EXIT_FAILURE);
	}

	// init inotify
	int fileDescriptor = inotify_init();
	printf("The file descriptor is: %d\n", fileDescriptor);

	if (fileDescriptor == -1) {
		perror("Unable to initialize inotify instance");
		exit(EXIT_FAILURE);
	}

	// track each file/directory
	char *watchFiles[argc];
	int added = 0;

	for (int i = 1; i < argc; ++i) {

		// add file/directory to inotify watch list
		if (inotify_add_watch(fileDescriptor, argv[i], IN_ALL_EVENTS) == -1) {
			fprintf(stderr, "Unable to add: %s to watch list\n", argv[i]);
			if (i + 1 == argc) {
				exit(EXIT_FAILURE);
			}
			continue;
		}
		watchFiles[added++] = argv[i];	// save for lookup
	}

	printf("\n++++++++Watching+++++++\n\n");
	for (int i = 0; i < added; ++i) {
		printf("%d: %s\n", i, watchFiles[i]);
	}
	printf("\n--------Daemon has started--------\n");

	daemonize(fileDescriptor, watchFiles);

	if (close(fileDescriptor)) {
		printf("error closing the file descriptor\n");
	}

	// free(watchFiles);
	return 0;
}

/**
 * function to daemonize the program
 *
 */

void daemonize(int fd, char **watchFiles) {

	char *message, *event;
	int length = -1;

	struct inotify_event *events;

	char buffer[BUFFER_SIZE];
	message = event = NULL;

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

				event = " was accessed";
				message = "The File has been accessed\n";
			}

			if (events->mask & IN_DELETE) {

				event = " was Deleted";
				message = "The File has been Deleted\n";
			}

			if (events->mask & IN_MODIFY) {

				event = " was modified";
				message = "The File has been Modified \n";
			}

			if (events->mask & IN_CREATE) {
				event = "File/Dir created";
				message = "File/Directory created";
			}
			if (events->mask & IN_CLOSE_WRITE) {
				event = "  write/close";
				message = "The file was written to and closed \n";
			}

			// increment eventPtr position in the buffer
			long eventSize = sizeof(struct inotify_event) + events->len;
			eventPtr += eventSize;
		}

		// if a directory is provided to track
		if (events->len > 0) {
			char path[400];
			snprintf(path, sizeof(path), "%s/%s", watchFiles[events->wd - 1], events->name);

			sendNotification(message, path, event);
		} else {
			char *file = watchFiles[events->wd - 1];
			sendNotification(message, file, event);
		}
	}
}

void sendNotification(char *message, char *fileName, char *event) {

	// format the notification title
	char notificationTitle[NAME_MAX + 100];
	snprintf(notificationTitle, sizeof(notificationTitle), "%s: %s", fileName, event);

	if (!notify_init("File Tracker")) {
		fprintf(stderr, "Error initialing libnotify instance\n");
		exit(EXIT_FAILURE);
	}

	NotifyNotification *notification = notify_notification_new(notificationTitle, message, NULL);
	notify_notification_set_urgency(notification, NOTIFY_URGENCY_CRITICAL);

	notify_notification_set_timeout(notification, 9000);
	notify_notification_show(notification, NULL);
}