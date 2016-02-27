#include <stdio.h>
#include <stdlib.h>

#include <xdo.h>

#include <linux/joystick.h>

#include "definitions.h"
#include "mainloop.h"

void mainloop(xdo_t* xdo, int joyfd, struct js_event* joystick, char jsbuttons, char jsaxes, char* buttons, char* axes, char* reversed, command* button_commands, command* axis_commands) {
	int last_value = 999999999;
	int delta;
	while (1) {
		if (read(joyfd, joystick, sizeof(joystick)) < 0)
			ewc(EXIT_FAILURE, "Failed to read from the joystick.");

		if (joystick->type == JS_EVENT_AXIS) {
			delta = reversed[joystick->number] * (joystick->value - last_value);
			switch (axis_commands[joystick->number].type) {
				case CMD_NONE:
				break;

				case CMD_MOUSEMOVE:
				break;

				case CMD_MOUSECLICK:
				if (strcmp(axis_commands[joystick->number].arguments[1], "current") == 0) {
					if (delta > 0) {
						if (strcmp(axis_commands[joystick->number].arguments[0], "left") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 1);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "middle") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 2);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "right") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 3);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "wheelup") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 4);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "wheeldown") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 5);
					} else {
						if (strcmp(axis_commands[joystick->number].arguments[0], "left") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 1);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "middle") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 2);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "right") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 3);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "wheelup") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 4);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "wheeldown") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 5);
					}
				} else {
					int x = atoi(axis_commands[joystick->number].arguments[1]);
					int y = atoi(axis_commands[joystick->number].arguments[2]);
					int screen = atoi(axis_commands[joystick->number].arguments[3]);
					xdo_move_mouse(xdo, x, y, screen);
					if (delta > 0) {
						if (strcmp(axis_commands[joystick->number].arguments[0], "left") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 1);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "middle") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 2);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "right") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 3);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "wheelup") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 4);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "wheeldown") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 5);
					} else {
						if (strcmp(axis_commands[joystick->number].arguments[0], "left") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 1);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "middle") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 2);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "right") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 3);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "wheelup") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 4);
						else if (strcmp(axis_commands[joystick->number].arguments[0], "wheeldown") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 5);
					}
				}
				break;

				case CMD_MOUSETELEPORT:
				if (delta > 0) {
					int x = atoi(axis_commands[joystick->number].arguments[0]);
					int y = atoi(axis_commands[joystick->number].arguments[1]);
					int screen = atoi(axis_commands[joystick->number].arguments[2]);
					xdo_move_mouse(xdo, x, y, screen);
				}
				break;

				case CMD_KEYPRESS:
				if (delta > 0)
					xdo_send_keysequence_window_down(xdo, CURRENTWINDOW, axis_commands[joystick->number].arguments[0], 0);
				else
					xdo_send_keysequence_window_up(xdo, CURRENTWINDOW, axis_commands[joystick->number].arguments[0], 0);
				break;

				case CMD_KEYSTROKE:
				if (delta > 0)
					xdo_enter_text_window(xdo, CURRENTWINDOW, axis_commands[joystick->number].arguments[0], 0);
				break;

				case CMD_COMMAND:
				if (delta > 0)
					system(axis_commands[joystick->number].arguments[0]);
				break;

				default:
				break;
			}
		} else if (joystick->type == JS_EVENT_BUTTON) {
			switch (button_commands[joystick->number].type) {
				case CMD_NONE:
				break;

				case CMD_MOUSECLICK:
				if (strcmp(button_commands[joystick->number].arguments[1], "current") == 0) {
					if (joystick->value == 1) {
						if (strcmp(button_commands[joystick->number].arguments[0], "left") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 1);
						else if (strcmp(button_commands[joystick->number].arguments[0], "middle") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 2);
						else if (strcmp(button_commands[joystick->number].arguments[0], "right") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 3);
						else if (strcmp(button_commands[joystick->number].arguments[0], "wheelup") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 4);
						else if (strcmp(button_commands[joystick->number].arguments[0], "wheeldown") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 5);
					} else {
						if (strcmp(button_commands[joystick->number].arguments[0], "left") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 1);
						else if (strcmp(button_commands[joystick->number].arguments[0], "middle") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 2);
						else if (strcmp(button_commands[joystick->number].arguments[0], "right") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 3);
						else if (strcmp(button_commands[joystick->number].arguments[0], "wheelup") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 4);
						else if (strcmp(button_commands[joystick->number].arguments[0], "wheeldown") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 5);
					}
				} else {
					int x = atoi(button_commands[joystick->number].arguments[1]);
					int y = atoi(button_commands[joystick->number].arguments[2]);
					int screen = atoi(button_commands[joystick->number].arguments[3]);
					xdo_move_mouse(xdo, x, y, screen);
					if (joystick->value == 1) {
						if (strcmp(button_commands[joystick->number].arguments[0], "left") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 1);
						else if (strcmp(button_commands[joystick->number].arguments[0], "middle") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 2);
						else if (strcmp(button_commands[joystick->number].arguments[0], "right") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 3);
						else if (strcmp(button_commands[joystick->number].arguments[0], "wheelup") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 4);
						else if (strcmp(button_commands[joystick->number].arguments[0], "wheeldown") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 5);
					} else {
						if (strcmp(button_commands[joystick->number].arguments[0], "left") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 1);
						else if (strcmp(button_commands[joystick->number].arguments[0], "middle") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 2);
						else if (strcmp(button_commands[joystick->number].arguments[0], "right") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 3);
						else if (strcmp(button_commands[joystick->number].arguments[0], "wheelup") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 4);
						else if (strcmp(button_commands[joystick->number].arguments[0], "wheeldown") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 5);
					}
				}
				break;

				case CMD_MOUSETELEPORT:
				if (joystick->value == 1) {
					int x = atoi(button_commands[joystick->number].arguments[0]);
					int y = atoi(button_commands[joystick->number].arguments[1]);
					int screen = atoi(button_commands[joystick->number].arguments[2]);
					xdo_move_mouse(xdo, x, y, screen);
				}
				break;

				case CMD_KEYPRESS:
				if (joystick->value == 1)
					xdo_send_keysequence_window_down(xdo, CURRENTWINDOW, button_commands[joystick->number].arguments[0], 0);
				else
					xdo_send_keysequence_window_up(xdo, CURRENTWINDOW, button_commands[joystick->number].arguments[0], 0);
				break;

				case CMD_KEYSTROKE:
				if (joystick->value == 1)
					xdo_enter_text_window(xdo, CURRENTWINDOW, button_commands[joystick->number].arguments[0], 0);
				break;

				case CMD_COMMAND:
				if (joystick->value == 1)
					system(button_commands[joystick->number].arguments[0]);
				break;

				default:
				break;
			}
		}

		last_value = joystick->value;
	}
}