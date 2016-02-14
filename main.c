#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/poll.h>

#include <linux/joystick.h>

static struct js_event joystick;
static int joyfd;

static pid_t pid, sid;

static char *confile, *mapfile, *joyfile;

static short buttons[19];
static short reverted[6];
static float sensitivity, frequency;

static char *mapping[19];

void arguments(int argc, char *argv[]);

void read_mapping();

void read_configuration();

void map_buttons_axes();

void daemon_create();

void mainLoop();

int main(int argc, char *argv[]) {
	arguments(argc, argv);

	read_mapping();

	if (!read_configuration())
		map_buttons_axes();

	daemon_create();
//	sleep(1);
/*
	char *buffer = (char*)malloc(128);
	char *info = (char*)malloc(2048);
	char *jsname = (char*)malloc(256);
	char jsaxes, jsbuttons;
	int jsversion;

	ioctl(joyfd, JSIOCGAXES, &jsaxes);
	ioctl(joyfd, JSIOCGBUTTONS, &jsbuttons);
	ioctl(joyfd, JSIOCGVERSION, &jsversion);
	ioctl(joyfd, JSIOCGNAME(256), jsname);
	sprintf(info, "notify-send \"AXES: %d, BUTTONS: %d, VERSION: %d, NAME: %s.\"", jsaxes, jsbuttons, jsversion, jsname);
	system(info);
*/
	main_loop();
}

void arguments(int argc, char *argv[]) {
	char option;

	if ((confile = malloc(strlen("undefined\n") + 4)) == NULL) {
		fprintf(stderr, "malloc() error.\n");
		exit(EXIT_FAILURE);
	}
	if ((mapfile = malloc(strlen("undefined\n") + 4)) == NULL) {
		fprintf(stderr, "malloc() error.\n");
		exit(EXIT_FAILURE);
	}
	if ((joyfile = malloc(strlen("undefined\n") + 4)) == NULL) {
		fprintf(stderr, "malloc() error.\n");
		exit(EXIT_FAILURE);
	}
	strcpy(confile, "\nundefined\n");
	strcpy(mapfile, "\nundefined\n");
	strcpy(joyfile, "\nundefined\n");

	while ((option = getopt(argc, argv, "m:j:c:hv")) != -1) {
		switch(option) {
			case 'm':
			if (strlen(optarg) > 1023) {
				fprintf(stderr, "Argument %c longer than 1023 characters.\n", optopt);
				exit(EXIT_FAILURE);
			}
			else
				if ((mapfile = realloc(strlen(optarg) + 4)) == NULL) {
					fprintf(stderr, "realloc() error.\n");
					exit(EXIT_FAILURE);
				}
				else
					strcpy(mapfile, optarg);
			break;

			case 'j':
			if (strlen(optarg) > 1023) {
				fprintf(stderr, "Argument %c longer than 1023 characters.\n", optopt);
				exit(EXIT_FAILURE);
			}
			else
				if ((joyfile = realloc(strlen(optarg) + 4)) == NULL) {
					fprintf(stderr, "realloc() error.\n");
					exit(EXIT_FAILURE);
				}
				else
					strcpy(joyfile, optarg);
			break;

			case 'c':
			if (strlen(optarg) > 1023) {
				fprintf(stderr, "Argument %c longer than 1023 characters\n", optopt);
				exit(EXIT_FAILURE);
			}
			else
				if ((confile = realloc(strlen(optarg) + 4)) == NULL) {
					fprintf(stderr, "realloc() error.\n");
					exit(EXIT_FAILURE);
				}
				else
					strcpy(confile, optarg);
			break;

			case 'h':
			fprintf(stdout, "Usage: %s [-m optional_file_with_mapping] [-j optional_argument_for_joystick_default_/dev/input/js0] 
				[-c optional_file_with_configuration] [-h] [-v]\n", __FILE__);
			exit(EXIT_SUCCESS);
			break;

			case 'v':
			fprintf(stdout, "Version 1.0.\n");
			exit(EXIT_SUCCESS);
			break;

			case ':':
			fprintf(stderr, "Argument %c requires an operand.\n", optopt);
			exit(EXIT_FAILURE);
			break;

			case '?':
			fprintf(stderr, "Unrecognized option: %c.\n", optopt);
			exit(EXIT_FAILURE);
			break;
		}
	}
}

void read_mapping() {
	if (!strcpy(mapfile, "\nundefined\n")) {
		fprintf(stderr, "The mapping file should be given.\n");
		exit(EXIT_FAILURE);
	}

	FILE *mapFile;
	char tempLine[1024];
	char tempToken[1024];

	if ((mapFile = fopen(mapfile, "r")) == NULL) {
		fprintf(stderr, "Failed to open the map file.\n");
		exit(EXIT_FAILURE);
	}

	enum {COM, ARG0, ARG1, ARGF} stages;
	enum stages tempStages = COM;

	short tempit;
	for (tempit = 0; tempit < 19; ++tempit) {
		tempLine = fgets(tempLine, sizeof(tempLine), mapFile);
		if (tempLine == NULL)
			break;
		else {
			while ((tempToken = strtok(tempLine, " ")) != NULL) {
				switch (tempStages) {
					case COM:
					if (!strcmp(tempLine, "undefined")) {
						mapping[tempit] = (char*)malloc(strlen(tempLine) + 1);
						strcpy(mapping[tempit], tempLine);
					} else if (!strcmp(tempLine, "keystroke")) {

					} else if (!strcmp(tempLine, "keypress")) {

					} else if (!strcmp(tempLine, "command")) {

					} else if (!strcmp(tempLine, "mouseclick")) {

					} else if (!strcmp(tempLine, "mousemove")) {
/*
					} else if (!strcmp(tempLine, "")) {
*/
					} else {
						fprintf(stderr, "Unrecognized command: %s.\nTerminating.\n", tempLine);
						exit(EXIT_FAILURE);
					}
					tempStages = ARG0;
					break;

					case ARG0:
					tempStages = ARG1;
					break;

					case ARG1:
					tempStages = ARGF;
					break;

					case ARGF:
					break;

					default:
					break;
				}
			}
		}

		tempStages = COM;
	}

	fclose(mapFile);
}

void map_buttons_axes() {
	for (short i = 0; i < 19; ++i)
		buttons[i] = -1;

	struct pollfd mtp[2];
	mtp[0].fd = 0;
	mtp[0].events = POLLIN;
	mtp[1].fd = joyfd;
	mtp[1].events = POLLIN;

	char minibuf[4];

	fprintf(stdout, "Mapping process has started! It'll help me finding out the proper codes for your gamepad! If no such button exists, press any key to skip.\nRelease all buttons and ")
	fprintf(stdout, "Press the up action button!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[0] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[0])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the right action button!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[1] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[1])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the down action button!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[2] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[2])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the left action button!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[3] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[3])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the up arrow!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[4] = joystick.number;
		if (joystick.value < 0)
			reverted[0] = 1;
		while (joystick.value != 0 && joystick.number == buttons[4])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the right arrow!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[5] = joystick.number;
		if (joystick.value < 0)
			reverted[1] = 1;
		while (joystick.value != 0 && joystick.number == buttons[5])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the L1!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[6] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[6])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the L2!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[7] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[7])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the L3!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[8] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[8])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the R1!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[9] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[9])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the R2!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[10] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[10])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the R3\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[11] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[11])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the SELECT button!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[12] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[12])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the START button!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[13] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[13])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Move the left analog vertically!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[14] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[14])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Move the left analog horizontally!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[15] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[15])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Move the right analog vertically!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[16] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[16])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Move the right analog horizontally!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[17] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[17])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Press the Home/Connect/any sort of big button in the middle, if it exists!\n");
	if (poll(mtp, 2, -1) < 0) {
		fprintf(stderr, "Error in poll() function:\n");
		fprintf(stderr, explain_poll(mtp, 2, -1));
		exit(EXIT_FAILURE);
	}
	if (mtp[0].revents & POLLIN)
		read(0, minibuf, sizeof(minibuf));
	else if (mtp[1].revents & POLLIN) {
		read(joyfd, &joystick, sizeof(joystick));
		buttons[18] = joystick.number;
		while (joystick.value != 0 && joystick.number == buttons[18])
			read(joyfd, &joystick, sizeof(joystick));
	}

	fprintf(stdout, "Mapping is finished! Would you like to save the configuration in a file for future usage? [Yy/Nn]\n")
	int conf = getchar();
	if (conf == 'Y' || conf == 'y') {
		while ((conf = getchar()) != '\n' && conf != EOF);
		fprintf(stdout, "Under the name of (no more than 63 characters): ");
		char *filename = (char*)malloc(64);
		if (getline(filename, (int*)(sizeof(filename) - 1), stdin) == -1) {
			fprintf(stderr, "Failed to get the filename string.\n");
			exit(EXIT_FAILURE);
		}

		if (access(filename, F_OK) == 0) {
			fprintf(stdout, "Are you sure you want to overwrite %s? [Yy/Nn]\n", filename);
			conf = getchar();
			if (conf == 'Y' || conf == 'y') {
				while ((conf = getchar()) != '\n' && conf != EOF);
				FILE *saveFile = fopen(filename, "w");
				for (short i = 0; i < 19; ++i)
					fprintf(saveFile, "%d ", buttons[i]);
				fprintf(saveFile, "\n");
				for (short i = 0; i < 6; ++i)
					fprintf(saveFile, "%d ", reverted[i]);

				fclose(saveFile);
			}
		} else {
			FILE *saveFile = fopen(filename, "w");
			for (short i = 0; i < 19; ++i)
				fprintf(saveFile, "%d ", buttons[i]);
			fprintf(saveFile, "\n");
			for (short i = 0; i < 6; ++i)
				fprintf(saveFile, "%d ", reverted[i]);

			fclose(saveFile);
		}

		free(filename);
	}
}

int read_configuration() {
	if (!strcmp(confile, "\nundefined\n"))
		return 0;

	memset(buttons, '\0', sizeof(buttons));
	memset(reverted, '\0', sizeof(reverted));

	FILE *conFile;
	if ((conFile = fopen(confile, "w")) == NULL) {
		fprintf(stderr, "Failed to open the configuration file.\n");
		exit(EXIT_FAILURE);
	}

	short it;
	short dummy;
	for (it = 0; it < 19; ++it) {
		short tempRet;
		if ((tempRet = fscanf(conFile, "%d\n", &dummy)) == EOF) {
			fprintf(stderr, "Unexpected EOF.");
			exit(EXIT_FAILURE);
		} else if (tempRet != 1 || dummy < 0 || dummy > 100) {
			fprintf(stderr, "This is not an appropriate configuration file.\n");
			exit(EXIT_FAILURE);
		}
		else
			buttons[it] = dummy;
	}
	for (it = 0; it < 6; ++it) {
		short tempRet;
		if ((tempRet = fscanf(conFile, "%d\n", &dummy)) == EOF) {
			fprintf(stderr, "Unexpected EOF.");
			exit(EXIT_FAILURE);
		} else if (tempRet != 1 || dummy < 0 || dummy > 100) {
			fprintf(stderr, "This is not an appropriate configuration file.\n");
			exit(EXIT_FAILURE);
		}
		else
			reverted[it] = dummy;
	}

	return 1;
}

void daemon_create() {
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
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	umask(027);

//	syslog(LOG_INFO, "Daemon %d has started.", pid);
}

void main_loop() {
	while (1) {
		if (read(joyfd, &joystick, sizeof(joystick)) < 0) {
			close(joyfd);
			system("notify-send \"Failed on reading.\"");
			exit(EXIT_FAILURE);
		} else {
			if (joystick.type == JS_EVENT_BUTTON) {
				sprintf(buffer, "notify-send \"Button event, value: %d, number: %d, time: %d.\"", joystick.value, joystick.number, joystick.time);
				system(buffer);
			}
		}
	}
/*
	char* temp = mapping[number];
	if (!strcmp(temp, "mousemove v")) {
		
	}

	else if (!strcmp(temp, "mousemove v")) {
		
	}

	else if (!strcmp(temp, "mousemove v")) {
		
	}

	else if (!strcmp(temp, "mousemove v")) {
		
	}

	else if (!strcmp(temp, "mousemove v")) {
		
	}

	else if (!strcmp(temp, "mousemove v")) {
		
	}
	
	else if (!strcmp(temp, "mousemove v")) {
		
	}
	
	else if 
*/
}
