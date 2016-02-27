#include <stdio.h>
#include <stdlib.h>

#include "daemon.h"

void daemon_create() {
	pid_t pid, sid;

	if ((pid = fork()) > 0) {
		fprintf(stdout, "Created a child successfully, PID: %d.\n", pid);
		exit(EXIT_SUCCESS);
	}

	if ((sid = setsid()) < 0) {
		fprintf(stderr, "Granted a session ID unsuccessfully.\n");
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "Session ID: %d.\n", sid);

	fprintf(stdout, "Changing PWD to / ...\n");
	if ((chdir("/")) < 0) {
		fprintf(stderr, "Failed to change PWD to /.\n");
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "Closing FDs 0, 1, 2 ...\n");
	close(0);
	close(1);
	close(2);

	umask(027);

//	syslog(LOG_INFO, "Daemon %d has started.", pid);
}