#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define DBGS(TXT) fprintf(stderr, "%s\n", TXT)
#define DBGN(NUM) fprintf(stderr, "%d\n", NUM)

typedef enum {
	CMD_NONE,
	CMD_MOUSEMOVE,
	CMD_MOUSECLICK,
	CMD_MOUSETELEPORT,
	CMD_KEYPRESS,
	CMD_KEYSTROKE,
	CMD_COMMAND
} cmd_type;

typedef struct command {
	cmd_type type;
	char** arguments;
	char lowhigh;
} command;

#endif