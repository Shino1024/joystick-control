#ifndef READING_H
#define READING_H

void read_configuration(int joyfd, struct js_event* joystick, int jsbuttons, int jsaxes, char* confile, char* buttons, char* axes, char* reversed);

#endif