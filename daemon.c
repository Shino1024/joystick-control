#include <stdio.h>
#include <stdlib.h>

#include "functions.h"
#include "daemon.h"

void daemon_create() {
	pid_t pid, sid;

	if ((pid = fork()) > 0) {
		FILE* temp_pid;
		if ((temp_pid = fopen("/tmp/joystick-control-pid", "w")) == NULL)
			ewc(EXIT_FAILURE, "Failed to open the /tmp/joystick-control-pid for saving the session ID.");
		fprintf(temp_pid, "%d", pid);
		fclose(temp_pid);
		ewc(EXIT_SUCCESS, "Created a child successfully, PID: %d.", pid);
	}
	else if (pid < 0)
		ewc(EXIT_FAILURE, "Error: Created a child unsuccessfully.");

	if ((sid = setsid()) == -1)
		ewc(EXIT_FAILURE, "Granted a session ID unsuccessfully.");
	fprintf(stdout, "Session ID: %d.\n", sid);

	fprintf(stdout, "Changing PWD to / ...\n");
	if ((chdir("/")) < 0)
		ewc(EXIT_FAILURE, "Failed to change PWD to /.");

	fprintf(stdout, "Closing FDs 0, 1, 2 ...\n");
	close(0);
	close(1);
	close(2);

	umask(027);

//	syslog(LOG_INFO, "Daemon %d has started.", pid);
}