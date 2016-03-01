#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/joystick.h>

#include "definitions.h"
#include "mapping.h"

void map_buttons_axes(int joyfd, struct js_event* joystick, char jsbuttons, char jsaxes, char* jsname, char* buttons, char* axes, char* reversed) {
	fprintf(stdout, "%s\n", "Press the button on the joystick to init the configuration wizard!");
	int temp_stat;
	while ((temp_stat = read(joyfd, joystick, sizeof(struct js_event))) >= 0 && (joystick->type & JS_EVENT_INIT) != 0);
	if (temp_stat < 0)
		ewc(EXIT_FAILURE, "Error while reading from the joystick.");

	if (read(joyfd, joystick, sizeof(struct js_event)) < 0)
		ewc(EXIT_FAILURE, "Error while reading from the joystick.");

	short temp_negative;
	for (temp_negative = 0; temp_negative < jsbuttons; ++temp_negative)
		buttons[temp_negative] = -1;
	for (temp_negative = 0; temp_negative < jsaxes; ++temp_negative) {
		axes[temp_negative] = -1;
		reversed[temp_negative] = 1;
	}

	char i = 0;
	fprintf(stdout, "Press all of the %d buttons! Do NOT press any button again.\n", jsbuttons);
	while (i < jsbuttons) {
		if (read(joyfd, joystick, sizeof(struct js_event)) < 0)
			ewc(EXIT_FAILURE, "Error in read() function.\n");

		if (joystick->type != JS_EVENT_BUTTON)
			continue;

		char temp_check, cont = 0;
		for (temp_check = 0; temp_check < jsbuttons; ++temp_check)
			if (buttons[temp_check] == joystick->number) {
				fprintf(stderr, "%s\n", "This button was pressed already!");
				cont = 1;
			}
		if (cont)
			continue;

		if (joystick->value == 1) {
			buttons[i] = joystick->number;
			fprintf(stdout, "%s %d\n", "Pressed button code:", joystick->number);
		}

		while (joystick->number != buttons[i] || joystick->value != 0)
			read(joyfd, joystick, sizeof(struct js_event));

		++i;
	}

	fprintf(stdout, "Move all of the %d axes! Do NOT move any axis again. Try to move the analogs perfectly straight. Move them right and up, not left and down.\n", jsaxes);
	i = 0;
	while (i < jsaxes) {
		if (read(joyfd, joystick, sizeof(struct js_event)) < 0)
			ewc(EXIT_FAILURE, "Error in read() function.\n");

		if (joystick->type != JS_EVENT_AXIS)
			continue;

		char temp_check, cont = 0;
		for (temp_check = 0; temp_check < jsaxes; ++temp_check)
			if (axes[temp_check] == joystick->number) {
				fprintf(stderr, "%s\n", "This axis was moved already!");
				cont = 1;
			}
		if (cont)
			continue;

		if (joystick->value != 0) {
			axes[i] = joystick->number;
			fprintf(stdout, "%s %d\n", "Moved axis code:", joystick->number);
			if (joystick->value < 0) {
				reversed[i] = -1;
				fprintf(stdout, "%s\n", "Known as reversed.");
			}
		}

		while (joystick->number != axes[i] || joystick->value != 0)
			read(joyfd, joystick, sizeof(struct js_event));

		++i;
	}

	fprintf(stdout, "Configuration is finished! Would you like to save the configuration in a file for future usage? [Yy/Nn] ");
	int yesorno = getc(stdin);
	if (yesorno == 'Y' || yesorno == 'y') {
		while ((yesorno = getc(stdin)) != '\n' && yesorno != EOF);
		fprintf(stdout, "Under the name of (no more than 255 characters): ");
		char filename[256];
		if (fgets(filename, sizeof(filename), stdin) == NULL)
			ewc(EXIT_FAILURE, "Failed to get the filename string.");

		filename[strlen(filename) - 1] = '\0';

		FILE* temp = fopen(filename, "r");
		if (temp != NULL) {
			fclose(temp);
			fprintf(stdout, "Are you sure you want to overwrite %s? [Yy/Nn] ", filename);
			yesorno = getc(stdin);
			if (yesorno == 'Y' || yesorno == 'y') {
				while ((yesorno = getc(stdin)) != '\n' && yesorno != EOF);
				FILE *saveFile = fopen(filename, "w");
				fprintf(saveFile, "%s\n", jsname);
				for (i = 0; i < jsbuttons; ++i)
					fprintf(saveFile, "%d\n", buttons[i]);
				for (i = 0; i < jsaxes; ++i)
					fprintf(saveFile, "%d\n", axes[i]);
				for (i = 0; i < jsaxes; ++i)
					fprintf(saveFile, "%d\n", reversed[i]);

				fclose(saveFile);
			}
		} else {
			FILE *saveFile = fopen(filename, "w");
			fprintf(saveFile, "%s\n", jsname);
			for (i = 0; i < jsbuttons; ++i)
				fprintf(saveFile, "%d\n", buttons[i]);
			for (i = 0; i < jsaxes; ++i)
				fprintf(saveFile, "%d\n", axes[i]);
			for (i = 0; i < jsaxes; ++i)
				fprintf(saveFile, "%d\n", reversed[i]);

			fclose(saveFile);
		}
	}
}