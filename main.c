#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <signal.h>
#include <syslog.h>

#include <linux/joystick.h>

#include <xdo.h>

#include "definitions.h"
#include "functions.h"
#include "parser.h"
#include "mapping.h"
#include "mainloop.h"
#include "daemon.h"
#include "reading.h"

xdo_t* xdo;

void xdo_exit(int dummy) {
	if (xdo != NULL)
		xdo_free(xdo);
	ewc(EXIT_SUCCESS, "\nReturning from the signal...");
}

int main(int argc, char* argv[]) {
	signal(SIGHUP, xdo_exit);
	signal(SIGINT, xdo_exit);
	signal(SIGTERM, xdo_exit);

	if (argc == 2)
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
			ewc(EXIT_SUCCESS, "Usage: %s /path/to/the/joystick/file /path/to/the/mapping/file [/optional/path/to/the/configuration/file]", argv[0]);

	FILE* temp_check;
	if ((temp_check = fopen("/tmp/joystick-control-pid", "r")) != NULL) {
		fclose(temp_check);
		ewc(EXIT_FAILURE, "There is another joystick-control's instance running already.");
	}

	struct js_event joystick;
	int joyfd;

	char jsbuttons, jsaxes;

	pid_t pid, sid;

	char* confile, * mapfile, * joyfile;

	float sensitivity;

	if (argc < 3)
		ewc(EXIT_FAILURE, "Too few arguments. Usage: %s /path/to/the/joystick/file /path/to/the/mapping/file [/optional/path/to/the/configuration/file]", argv[0]);

	joyfile = argv[1];
	mapfile = argv[2];
	if (argc == 4)
		confile = argv[3];
	else
		confile = NULL;

	if ((joyfd = open(joyfile, O_RDONLY)) == -1)
		ewc(EXIT_FAILURE, "Unable to open the joystick file.");

	if (ioctl(joyfd, JSIOCGBUTTONS, &jsbuttons) == -1)
		ewc(EXIT_FAILURE, "Error returned by ioctl() while reading the number of buttons.");
	if (ioctl(joyfd, JSIOCGAXES, &jsaxes) == -1)
		ewc(EXIT_FAILURE, "Error returned by ioctl() while reading the number of axes.");
	char jsname[1024];
	if (ioctl(joyfd, JSIOCGNAME(1023), jsname) == -1)
		ewc(EXIT_FAILURE, "Error returned by ioctl() while reading the joystick's name.");
	fprintf(stdout, "%s %s\n", "Joystick name:", jsname);

	command button_commands[jsbuttons];
	command axis_commands[jsaxes];
	char buttons[jsbuttons];
	char axes[jsaxes];
	char reversed[jsaxes];

	char temp_filler;
	for (temp_filler = 0; temp_filler < jsbuttons; ++temp_filler) {
		command temp;
		temp.type = CMD_NONE;
		button_commands[temp_filler] = temp;
	}

	for (temp_filler = 0; temp_filler < jsaxes; ++temp_filler) {
		command temp;
		temp.type = CMD_NONE;
		axis_commands[temp_filler] = temp;
	}

	read_mapping(mapfile, joyfd, &joystick, jsbuttons, jsaxes, buttons, axes, reversed, button_commands, axis_commands);

	if (confile)
		read_configuration(joyfd, &joystick, jsbuttons, jsaxes, jsname, confile, buttons, axes, reversed);
	else
		map_buttons_axes(joyfd, &joystick, jsbuttons, jsaxes, jsname, buttons, axes, reversed);

	openlog("joystick-control", 0, LOG_USER);
	xdo = xdo_new(NULL);
	daemon_create();

	mainloop(xdo, joyfd, &joystick, jsbuttons, jsaxes, buttons, axes, reversed, button_commands, axis_commands);

	return 0;
}