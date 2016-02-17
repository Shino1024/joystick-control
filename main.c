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

static char* buttons;
static char* axes;
static char* reversed;
static float sensitivity, frequency;

// static char *mapping[21];

static char jsaxes, jsbuttons;

static int error;

int ewc(int code, const char* message);

int parse(int argc, char* argv[], char* f0, char* f1, char* f2);

void read_mapping();

void read_configuration();

void map_buttons_axes();

void daemon_create();

void main_loop();

int main(int argc, char *argv[]) {
	if (parse(argc, argv) != 0)
		ewc(EXIT_FAILURE, "Too few arguments.");

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

int ewc(int code, const char* message) {
	fprintf(stderr, "%s\n", message);
	exit(code);
}

int parse(int argc, char* argv[], char* f0, char* f1, char* f2) {
	if (argc < 4) {
		return 1;
	}
	strcpy(f0, argv[1]);
	strcpy(f1, argv[2]);
	if (argc == 4)
		strcpy(f2, argv[3]);
	else
		strcpy(f2, "undefined");
	return 0;
}
/*
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
*/
void map_buttons_axes() {
	ioctl(joyfd, JSIOCGBUTTONS, &jsbuttons);
	ioctl(joyfd, JSIOCGAXES, &jsaxes);

	buttons = (char*)calloc(sizeof(char) * jsbuttons);
	axes = (char*)calloc(sizeof(char) * jsaxes);
	reversed = (char*)calloc(sizeof(char) * jsaxes);

	fprintf(stdout, "%s\n", "Press the button on the joystick to init the configuration wizard!");
	while ((error = read(joyfd, &joystick, sizeof(joystick))) >= 0 && (joystick.type & JS_EVENT_INIT) != 0)
		if (error < 0)
			ewc(EXIT_FAILURE, "read() error.");
	if (read(joyfd, &joystick, sizeof(joystick)) < 0)
		ewc(EXIT_FAILURE, "read() error");

	char i = 0;
	fprintf(stdout, "Press all of the %d buttons! Do NOT press any button again.\n", jsbuttons);
	while (i < jsbuttons) {
		if (read(joyfd, &joystick, sizeof(joystick)) < 0)
			ewc(EXIT_FAILURE, "read() error.");

		if (joystick.type != JS_EVENT_BUTTON)
			continue;
		
		if (joystick.value == 1) {
			buttons[i] = joystick.number;
			fprintf(stdout, "%d\n", buttons[i]);
		}

		while (joystick.number != buttons[i] || joystick.value != 0)
			if (read(joyfd, &joystick, sizeof(joystick)) < 0)
				ewc(EXIT_FAILURE, "read() error.");

		++i;
	}

	fprintf(stdout, "Move all of the %d axes! Do NOT move any axis again. Try to move the analogs perfectly straight.\n", jsaxes);
	i = 0;
	while (i < jsaxes) {
		if (read(joyfd, &joystick, sizeof(joystick)) < 0)
			ewc(EXIT_FAILURE, "read() error.");

		if (joystick.type != JS_EVENT_AXIS)
			continue;

		if (joystick.value != 0) {
			axes[i] = joystick.number;
			fprintf(stdout, "%d\n", axes[i]);
			if (joystick.value < 0) {
				reversed[i] = 1;
				fprintf(stdout, "%s\n", "Information: this axis is reversed and this program will be interpreting it as such.");
			}
		}

		while (joystick.number != axes[i] || joystick.value != 0)
			if (read(joyfd, &joystick, sizeof(joystick)) < 0)
				ewc(EXIT_FAILURE, "read() error.");

		++i;
	}

	char i;
	fprintf(stdout, "Configuring is finished! Would you like to save the configuration in a file for future usage? [Yy/Nn]\n");
	int yesorno = getchar();
	if (yesorno == EOF)
		ewc(EXIT_FAILURE, "getchar() failed.");
	if (yesorno == 'Y' || yesorno == 'y') {
		while ((yesorno = getchar()) != '\n' && yesorno != EOF);
		fprintf(stdout, "Under the name of (no more than 63 characters): ");
		char filename[256];
		if (fgets(filename, sizeof(filename), stdin) == NULL)
			ewc(EXIT_FAILURE, "Failed to get the filename string.");

		filename[strlen(filename) - 1] = '\0';

		FILE* temp = fopen(filename, "r");
		if (temp != NULL) {
			fclose(temp);
			fprintf(stdout, "Are you sure you want to overwrite %s? [Yy/Nn]\n", filename);
			yesorno = getchar();
			if (yesorno == EOF)
				ewc(EXIT_FAILURE, "getchar() failed.");
			if (yesorno == 'Y' || yesorno == 'y') {
				while ((yesorno = getchar()) != '\n' && yesorno != EOF);
				FILE *saveFile = fopen(filename, "w");
				for (i = 0; i < sizeof(buttons); ++i)
					fprintf(saveFile, "%d\n", buttons[i]);
				for (i = 0; i < sizeof(axes); ++i)
					fprintf(saveFile, "%d\n", axes[i]);

				fclose(saveFile);
			}
		} else {
			FILE *saveFile = fopen(filename, "w");
			for (i = 0; i < sizeof(buttons); ++i)
				fprintf(saveFile, "%d\n", buttons[i]);
			for (i = 0; i < sizeof(axes); ++i)
				fprintf(saveFile, "%d\n", axes[i]);

			fclose(saveFile);
		}
	}
}

int my_atoi(const char* string) {
	if (*string == '-')
		return -1;
	else
		return atoi(string);
}

int read_configuration(const char* confile, int buttons[], int reversed[]) {
	if (!strcmp(confile, "\nundefined\n"))
		return 0;

	memset(buttons, '\0', sizeof(buttons));
	memset(reversed, '\0', sizeof(reversed));
	char tempLine[3];

	FILE *conFile;
	if ((conFile = fopen(confile, "ra")) == NULL) {
		fprintf(stderr, "Failed to open the conping file.\n");
		exit(EXIT_FAILURE);
	}

	short it;
	for (it = 0; it < 19; ++it) {
		if (fgets(tempLine, 3, conFile) == NULL) {
			fprintf(stderr, "Unexpected EOF.\n");
			exit(EXIT_FAILURE);
		} else if (my_atoi(tempLine) < -1 || my_atoi(tempLine) > 100) {
			fprintf(stderr, "This is not an appropriate conping file.\n");
			exit(EXIT_FAILURE);
		}
		else
			buttons[it] = my_atoi(tempLine);
	}
	for (it = 0; it < 6; ++it) {
		if (fgets(tempLine, 3, conFile) == NULL) {
			fprintf(stderr, "Unexpected EOF.\n");
			exit(EXIT_FAILURE);
		} else if (my_atoi(tempLine) < -1 || my_atoi(tempLine) > 100) {
			fprintf(stderr, "This is not an appropriate conping file.\n");
			exit(EXIT_FAILURE);
		}
		else
			reversed[it] = my_atoi(tempLine);
	}

	fclose(conFile);

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
