#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/joystick.h>

#include "definitions.h"
#include "functions.h"
#include "parser.h"

void read_mapping(char* mapfile, int joyfd, struct js_event* joystick, char jsbuttons, char jsaxes, char* buttons, char* axes, char* reversed, command* button_commands, command* axis_commands) {
	char line[10240];
	FILE* script;
	if ((script = fopen(mapfile, "r")) == NULL)
		ewc(EXIT_FAILURE, "Couldn't open the map file.");

	while (fgets(line, sizeof(line), script) != NULL) {
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';
		char* temp_line = strdup(line);

		char* token = strtok(line, " ");
		if (strcmp(token, "\n") == 0)
			continue;
		if (*token != 'a' && *token != 'b')
			ewc(EXIT_FAILURE, "Syntax error: The code begins neither with 'a', nor with 'b'.");
		char is_button = *token - 'a';
		token++;
		int temp_code = atoi(token);
		if (!is_button) {
			if (temp_code < 0 || temp_code > jsaxes - 1)
				ewc(EXIT_FAILURE, "Error: There is no %d axis present in the opened controller.", temp_code);
			if (axis_commands[temp_code].type != CMD_NONE)
				ewc(EXIT_FAILURE, "Error: Axis %d has a command assigned already.", temp_code);
		}
		else {
			if (temp_code < 0 || temp_code > jsbuttons - 1)
				ewc(EXIT_FAILURE, "Error: There is no %d button present in the opened controller.", temp_code);
			if (button_commands[temp_code].type != CMD_NONE)
				ewc(EXIT_FAILURE, "Error: Button %d has a command assigned already.", temp_code);
		}

		if ((token = strtok(NULL, " ")) == NULL)
			ewc(EXIT_FAILURE, "Syntax error: Found no command after a key/button code.");

		if (strcmp(token, "mousemove") == 0) {
			if (is_button)
				ewc(EXIT_FAILURE, "Error: A button can't have a mousemove command assigned to.");
			if ((token = strtok(NULL, " ")) == NULL)
				ewc(EXIT_FAILURE, "Syntax error: Found no argument after \"mousemove\".");
			if (strcmp(token, "horizontal") != 0 && strcmp(token, "vertical") != 0)
				ewc(EXIT_FAILURE, "Syntax error: The first argument of \"mousemove\" should be either \"vertical\" or \"horizontal\".");
			char* temp_arg_0 = strdup(token);

			if ((token = strtok(NULL, " \n")) == NULL)
				ewc(EXIT_FAILURE, "Syntax error: sensitivity level required as a second argument of \"mousemove\" [0-9].");
			char* temp_arg_1 = strdup(token);
			if (*temp_arg_1 < 0 || strlen(temp_arg_1) > 1)
				ewc(EXIT_FAILURE, "Syntax error: sensitivity level of \"mousemove\" should be between [0-9].");

			command temp;
			temp.type = CMD_MOUSEMOVE;
			temp.arguments = (char**)emalloc(sizeof(char*) * 2);
			temp.arguments[0] = strdup(temp_arg_0);
			temp.arguments[1] = strdup(temp_arg_1);
			axis_commands[temp_code] = temp;

			free(temp_arg_0);
			free(temp_arg_1);
		} else if (strcmp(token, "mouseclick") == 0) {
			if ((token = strtok(NULL, " \n")) == NULL)
				ewc(EXIT_FAILURE, "Syntax error: Found no argument after \"mouseclick\".");
			if (strcmp(token, "left") != 0 && strcmp(token, "middle") != 0 && strcmp(token, "right") != 0 && strcmp(token, "wheelup") != 0 && strcmp(token, "wheeldown") != 0)
				ewc(EXIT_FAILURE, "Syntax error: The first argument of \"mouseclick\" should be \"left\", \"middle\", \"right\", \"wheelup\" or \"wheeldown\".");
			char* temp_arg_0 = strdup(token);

			char* check;
			char* temp_arg_1, * temp_arg_2, * temp_arg_3;
			if ((token = strtok(NULL, " \n")) == NULL) {
				temp_arg_1 = strdup("current");
				temp_arg_2 = strdup("current");
			} else {
				check = token;
				while (*check != '\0') {
					if (*check < '0' || *check > '9')
						ewc(EXIT_FAILURE, "Syntax error: The second argument of \"mouseclick\" should be a number.");
					check++;
				}
				temp_arg_1 = strdup(token);

				if ((token = strtok(NULL, " \n")) == NULL)
					ewc(EXIT_FAILURE, "Provided only one coordinate instead of two in \"mouseclick\".");
				check = token;
				while (*check != '\0') {
					if (*check < '0' || *check > '9')
						ewc(EXIT_FAILURE, "Syntax error: The third argument of \"mouseclick\" should be a number.");
					check++;
				}
				temp_arg_2 = strdup(token);
			}

			if ((token = strtok(NULL, " \n")) == NULL)
				temp_arg_3 = strdup("0");
			else
				temp_arg_3 = strdup("current");

			command temp;
			temp.type = CMD_MOUSECLICK;
			temp.arguments = (char**)emalloc(sizeof(char*) * 4);
			temp.arguments[0] = strdup(temp_arg_0);
			temp.arguments[1] = strdup(temp_arg_1);
			temp.arguments[2] = strdup(temp_arg_2);
			temp.arguments[3] = strdup(temp_arg_3);
			if (!is_button)
				axis_commands[temp_code] = temp;
			else
				button_commands[temp_code] = temp;

			free(temp_arg_0);
			free(temp_arg_1);
			free(temp_arg_2);
		} else if (strcmp(token, "mouseteleport") == 0) {
			if ((token = strtok(NULL, " ")) == NULL)
				ewc(EXIT_FAILURE, "Syntax error: Found no argument after \"mouseteleport\". It should be an X coordinate.");
			char* check = token;
			while (*check != '\0') {
				if (*check < '0' || *check > '9')
					ewc(EXIT_FAILURE, "Syntax error: The first argument of \"mouseteleport\" should be a number.");
				check++;
			}
			char* temp_arg_0 = strdup(token);

			if ((token = strtok(NULL, " \n")) == NULL)
				ewc(EXIT_FAILURE, "Syntax error: The first argument of \"mouseteleport\" should be a Y coordinate.");
			check = token;
			while (*check != '\0') {
				if (*check < '0' || *check > '9')
					ewc(EXIT_FAILURE, "Syntax error: The second argument of \"mouseteleport\" should be a number.");
				check++;
			}
			char* temp_arg_1 = strdup(token);

			char* temp_arg_2 = NULL;
			if ((token = strtok(NULL, " \n")) != NULL) {
				check = token;
				while (*check != '\0') {
					if (*check < '0' || *check > '9')
						ewc(EXIT_FAILURE, "Syntax error: The third argument of \"mouseteleport\" should be a number.");
					check++;
				}
				temp_arg_2 = strdup(token);
			}

			command temp;
			temp.type = CMD_MOUSETELEPORT;
			temp.arguments = (char**)emalloc(sizeof(char*) * 3);
			temp.arguments[0] = strdup(temp_arg_0);
			temp.arguments[1] = strdup(temp_arg_1);
			if (temp_arg_2)
				temp.arguments[2] = strdup(temp_arg_2);
			else
				temp.arguments[2] = strdup("0");
			if (!is_button)
				axis_commands[temp_code] = temp;
			else
				button_commands[temp_code] = temp;

			free(temp_arg_0);
			free(temp_arg_1);
			free(temp_arg_2);
		} else if (strcmp(token, "keypress") == 0) {
			if ((token = strtok(NULL, " ")) == NULL)
				ewc(EXIT_FAILURE, "Syntax error: Found no argument after \"keypress\".");
			char* temp_arg_0 = strdup(token);
			if ((token = strtok(NULL, " \n")) != NULL)
				ewc(EXIT_FAILURE, "Syntax error: \"Keypress\" can press only one sequence of keys. Do not separate the sequences with the space, use the '+' instead.");

			command temp;
			temp.type = CMD_KEYPRESS;
			temp.arguments = (char**)emalloc(sizeof(char*));
			temp.arguments[0] = strdup(temp_arg_0);
			if (!is_button)
				axis_commands[temp_code] = temp;
			else
				button_commands[temp_code] = temp;

			free(temp_arg_0);
		} else if (strcmp(token, "keystroke") == 0) {
			if ((token = strtok(NULL, " ")) == NULL)
				ewc(EXIT_FAILURE, "Found no text to type after \"keystroke\".");

			char* line1 = strrep((char*)temp_line, "/NEWLINE", "\n");
			line1 = strrep(line1, "/HTAB", "\t");

			char* temp_token = strstr(line1, " ");
			temp_token++;
			temp_token = strstr(temp_token, " ");
			temp_token++;

			command temp;
			temp.type = CMD_KEYSTROKE;
			temp.arguments = (char**)emalloc(sizeof(char*));
			temp.arguments[0] = strdup(temp_token);
			if (!is_button)
				axis_commands[temp_code] = temp;
			else
				button_commands[temp_code] = temp;
		} else if (strcmp(token, "command") == 0) {
			if ((token = strtok(NULL, " ")) == NULL)
				ewc(EXIT_FAILURE, "Found no command to perform after \"command\".");

			char* line1 = strrep((char*)temp_line, "/NEWLINE", "\n");
			line1 = strrep(line1, "/HTAB", "\t");

			line1 = strstr(line1, " ");
			line1++;
			line1 = strstr(line1, " ");
			line1++;

			command temp;
			temp.type = CMD_COMMAND;
			temp.arguments = (char**)emalloc(sizeof(char*));
			temp.arguments[0] = strdup(line1);
			if (!is_button)
				axis_commands[temp_code] = temp;
			else
				button_commands[temp_code] = temp;
		} else 
			ewc(EXIT_FAILURE, "Syntax error: The command %s is not recognized.", token);
	}

	fclose(script);
}