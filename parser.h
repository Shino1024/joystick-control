#ifndef PARSER_H
#define PARSER_H

void read_mapping(char* mapfile, int joyfd, struct js_event* joystick, char jsbuttons, char jsaxes, char* buttons, char* axes, char* reversed, command* button_commands, command* axis_commands);

#endif