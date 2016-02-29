#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>

#include <xdo.h>

#include <linux/joystick.h>

#include "definitions.h"
#include "mainloop.h"

void mainloop(xdo_t* xdo, int joyfd, struct js_event* joystick, char jsbuttons, char jsaxes, char* buttons, char* axes, char* reversed, command* button_commands, command* axis_commands) {
	int last_value[jsaxes];
	char x;
	for (x = 0; x < jsaxes; ++x)
		last_value[x] = 0;

	char temp_axes[jsaxes];
	for (x = 0; x < jsaxes; ++x)
		temp_axes[axes[x]] = x;

	long delta[jsaxes];
	int sensitivity;
	while (1) {
		if (read(joyfd, joystick, sizeof(joystick)) < 0)
			syslog(LOG_ERR, "Failed to read from the joystick.");

		if (joystick->type == JS_EVENT_AXIS) {
			delta[joystick->number] = reversed[temp_axes[joystick->number]] * joystick->value - last_value[joystick->number];
			if (reversed[temp_axes[joystick->number]] * joystick->value > 0 && delta[joystick->number] < 0 || reversed[temp_axes[joystick->number]] * joystick->value < 0 && delta[joystick->number] > 0)
				continue;
			else if (joystick->value == 0) {
				last_value[joystick->number] = 0;
				continue;
			}

			switch (axis_commands[axes[joystick->number]].type) {
				case CMD_NONE:
				break;

				case CMD_MOUSEMOVE:
				sensitivity = atoi(axis_commands[joystick->number].arguments[1]);
				delta[joystick->number] = reversed[temp_axes[joystick->number]] * delta[joystick->number] * (delta[joystick->number] > 0 ? delta[joystick->number] : delta[joystick->number] * (-1)) * sensitivity / 1500000;
				if (strcmp(axis_commands[joystick->number].arguments[0], "horizontal") == 0)
					xdo_move_mouse_relative(xdo, delta[joystick->number], 0);
				else 
					xdo_move_mouse_relative(xdo, 0, delta[joystick->number]);
				break;

				case CMD_MOUSECLICK:
				if (strcmp(axis_commands[axes[joystick->number]].arguments[1], "current") == 0) {
					if (delta[joystick->number] > 0) {
						if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "left") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 1);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "middle") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 2);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "right") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 3);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "wheelup") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 4);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "wheeldown") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 5);
					} else {
						if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "left") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 1);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "middle") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 2);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "right") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 3);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "wheelup") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 4);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "wheeldown") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 5);
					}
				} else {
					int x = atoi(axis_commands[axes[joystick->number]].arguments[1]);
					int y = atoi(axis_commands[axes[joystick->number]].arguments[2]);
					int screen = atoi(axis_commands[axes[joystick->number]].arguments[3]);
					xdo_move_mouse(xdo, x, y, screen);
					if (delta[joystick->number] > 0) {
						if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "left") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 1);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "middle") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 2);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "right") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 3);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "wheelup") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 4);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "wheeldown") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 5);
					} else {
						if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "left") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 1);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "middle") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 2);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "right") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 3);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "wheelup") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 4);
						else if (strcmp(axis_commands[axes[joystick->number]].arguments[0], "wheeldown") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 5);
					}
				}
				break;

				case CMD_MOUSETELEPORT:
				if (delta[joystick->number] > 0) {
					int x = atoi(axis_commands[axes[joystick->number]].arguments[0]);
					int y = atoi(axis_commands[axes[joystick->number]].arguments[1]);
					int screen = atoi(axis_commands[axes[joystick->number]].arguments[2]);
					xdo_move_mouse(xdo, x, y, screen);
				}
				break;

				case CMD_KEYPRESS:
				if (delta[joystick->number] > 0)
					xdo_send_keysequence_window_down(xdo, CURRENTWINDOW, axis_commands[axes[joystick->number]].arguments[0], 0);
				else
					xdo_send_keysequence_window_up(xdo, CURRENTWINDOW, axis_commands[axes[joystick->number]].arguments[0], 0);
				break;

				case CMD_KEYSTROKE:
				if (delta[joystick->number] > 0)
					xdo_enter_text_window(xdo, CURRENTWINDOW, axis_commands[axes[joystick->number]].arguments[0], 0);
				break;

				case CMD_COMMAND:
				if (delta[joystick->number] > 0)
					system(axis_commands[axes[joystick->number]].arguments[0]);
				break;

				default:
				break;
			}
		} else if (joystick->type == JS_EVENT_BUTTON) {
			switch (button_commands[buttons[joystick->number]].type) {
				case CMD_NONE:
				break;

				case CMD_MOUSECLICK:
				if (strcmp(button_commands[buttons[joystick->number]].arguments[1], "current") == 0) {
					if (joystick->value == 1) {
						if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "left") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 1);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "middle") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 2);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "right") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 3);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "wheelup") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 4);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "wheeldown") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 5);
					} else {
						if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "left") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 1);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "middle") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 2);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "right") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 3);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "wheelup") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 4);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "wheeldown") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 5);
					}
				} else {
					int x = atoi(button_commands[buttons[joystick->number]].arguments[1]);
					int y = atoi(button_commands[buttons[joystick->number]].arguments[2]);
					int screen = atoi(button_commands[buttons[joystick->number]].arguments[3]);
					xdo_move_mouse(xdo, x, y, screen);
					if (joystick->value == 1) {
						if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "left") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 1);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "middle") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 2);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "right") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 3);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "wheelup") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 4);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "wheeldown") == 0)
							xdo_mouse_down(xdo, CURRENTWINDOW, 5);
					} else {
						if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "left") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 1);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "middle") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 2);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "right") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 3);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "wheelup") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 4);
						else if (strcmp(button_commands[buttons[joystick->number]].arguments[0], "wheeldown") == 0)
							xdo_mouse_up(xdo, CURRENTWINDOW, 5);
					}
				}
				break;

				case CMD_MOUSETELEPORT:
				if (joystick->value == 1) {
					int x = atoi(button_commands[buttons[joystick->number]].arguments[0]);
					int y = atoi(button_commands[buttons[joystick->number]].arguments[1]);
					int screen = atoi(button_commands[buttons[joystick->number]].arguments[2]);
					xdo_move_mouse(xdo, x, y, screen);
				}
				break;

				case CMD_KEYPRESS:
				if (joystick->value == 1)
					xdo_send_keysequence_window_down(xdo, CURRENTWINDOW, button_commands[buttons[joystick->number]].arguments[0], 0);
				else
					xdo_send_keysequence_window_up(xdo, CURRENTWINDOW, button_commands[buttons[joystick->number]].arguments[0], 0);
				break;

				case CMD_KEYSTROKE:
				if (joystick->value == 1)
					xdo_enter_text_window(xdo, CURRENTWINDOW, button_commands[buttons[joystick->number]].arguments[0], 0);
				break;

				case CMD_COMMAND:
				if (joystick->value == 1)
					system(button_commands[buttons[joystick->number]].arguments[0]);
				break;

				default:
				break;
			}
		}

		last_value[joystick->number] = reversed[temp_axes[joystick->number]] * joystick->value;
	}
}