#ifndef MAPPING_H
#define MAPPING_H

void map_buttons_axes(int joyfd, struct js_event* joystick, char jsbuttons, char jsaxes, char* buttons, char* axes, char* reversed);

#endif