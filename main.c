#include <fcntl.h>
#include <gio/gio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#define NAME_MAX 255  // max name for a file
#define BUFFER_SIZE sizeof(struct inotify_event) + NAME_MAX + 1

void sendNotification(char *message, char *fileName, char *event);

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Please provide a file to track \n");

		exit(EXIT_FAILURE);
	}

	char *filePath = argv[1];
	char buffer[4090];
	ssize_t length;

	struct inotify_event *events;

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
	printf("Waiting for events to happen 1\n");

	char *message, *file, *event;
	message = file = event = NULL;
	while (1) {

		length = read(fileDescriptor, buffer, sizeof(buffer));

		if (length <= 0) {
			perror("Error reading the file descriptor");
			break;	// TODO: why break
		}

		for (char *bufferPtr = buffer; bufferPtr < buffer + length;
			 bufferPtr += sizeof(struct inotify_event) + events->len) {

			events = (struct inotify_event *)bufferPtr;

			if (events->mask & IN_ACCESS) {

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

			if (events->len > 0) {
				printf("THe file name is: %s\n", events->name);
			}
			file = events->name;
		}

		if (message) {
			sendNotification(message, file, event);
		}
	}

	// i += sizeof(struct inotify_event) + events->len;

	if (close(fileDescriptor)) {
		printf("error closing the file descriptor\n");
	}
	return 0;
}

/**
 * The implementation of this function was provided by
 *<https://wiki.archlinux.org/title/Desktop_notifications
 */

void sendNotification(char *message, char *fileName, char *event) {

	// format the notification title
	char notificationTitle[NAME_MAX + 100];
	snprintf(notificationTitle, sizeof(notificationTitle), "%s: %s", fileName, event);

	GApplication *application = g_application_new("hello.world", G_APPLICATION_DEFAULT_FLAGS);
	g_application_register(application, NULL, NULL);
	GNotification *notification = g_notification_new(notificationTitle);
	g_notification_set_body(notification, message);

	g_notification_set_priority(notification, G_NOTIFICATION_PRIORITY_HIGH);
	GIcon *icon = g_themed_icon_new("dialog-warning");
	g_notification_set_icon(notification, icon);
	g_application_send_notification(application, NULL, notification);
	g_object_unref(icon);
	g_object_unref(notification);
	g_object_unref(application);
}