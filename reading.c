#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/joystick.h>

#include "definitions.h"
#include "reading.h"

void read_configuration(int joyfd, struct js_event* joystick, char jsbuttons, char jsaxes, char* jsname, char* confile, char* buttons, char* axes, char* reversed) {
	char tempLine[4];

	FILE *conFile;
	if ((conFile = fopen(confile, "r")) == NULL)
		ewc(EXIT_FAILURE, "Unable to open the configuration file.");

	fprintf(stdout, "%s\n", "Press any button on the joystick to begin...");
	int temp_stat;
	while ((temp_stat = read(joyfd, joystick, sizeof(struct js_event))) >= 0 && (joystick->type & JS_EVENT_INIT) != 0);
	if (temp_stat < 0)
		ewc(EXIT_FAILURE, "Error while reading from the joystick.");

	if (read(joyfd, joystick, sizeof(struct js_event)) < 0)
		ewc(EXIT_FAILURE, "Error while reading from the joystick.");

	fprintf(stdout, "%s\n", "Reading configuration...");
	char file_jsname[1024];
	if (fgets(file_jsname, 1024, conFile) == NULL)
		ewc(EXIT_FAILURE, "Unexpected EOF. Is it an appropriate configuration file? I am expecting a joystick's name here.");

	if (file_jsname[strlen(file_jsname) - 1] == '\n')
		file_jsname[strlen(file_jsname) - 1] = '\0';
	if (strcmp(jsname, file_jsname) != 0)
		ewc(EXIT_FAILURE, "Error: This configuration file wasn't made for that joystick.");

	short it;
	for (it = 0; it < jsbuttons; ++it) {
		if (fgets(tempLine, 4, conFile) == NULL)
			ewc(EXIT_FAILURE, "Unexpected EOF. Is it an appropriate configuration file? I am expecting a button code number here.");
		if (atoi(tempLine) < -1 || atoi(tempLine) > 99)
			ewc(EXIT_FAILURE, "Inappropriate configuration file!");
		else
			buttons[it] = atoi(tempLine);
	}

	for (it = 0; it < jsaxes; ++it) {
		if (fgets(tempLine, 4, conFile) == NULL)
			ewc(EXIT_FAILURE, "Unexpected EOF. Is it an appropriate configuration file? I am expecting an axis code number here.");
		if (atoi(tempLine) < -1 || atoi(tempLine) > 99)
			ewc(EXIT_FAILURE, "Inappropriate configuration file!");
		else
			axes[it] = atoi(tempLine);
	}


	for (it = 0; it < jsaxes; ++it) {
		if (fgets(tempLine, 4, conFile) == NULL)
			ewc(EXIT_FAILURE, "Unexpected EOF. Is it an appropriate configuration file? I am expecting an axis reversal information here.");
		if (atoi(tempLine) != -1 && atoi(tempLine) != 1)
			ewc(EXIT_FAILURE, "Inappropriate configuration file! Expecting either -1 or 1 here.");
		else
			reversed[it] = atoi(tempLine);
	}

	fprintf(stdout, "%s\n", "Configuration read successfully.");
	fclose(conFile);
}