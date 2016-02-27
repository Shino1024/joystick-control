#ifndef MAINLOOP_H
#define MAINLOOP_H

void mainloop(xdo_t* xdo, int joyfd, struct js_event* joystick, char jsbuttons, char jsaxes, char* buttons, char* axes, char* reversed, command* button_commands, command* axis_commands);

#endif