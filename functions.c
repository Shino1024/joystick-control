#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <linux/joystick.h>

#include "functions.h"

void* emalloc(size_t size) {
	void* ret = malloc(size);
	if (ret == NULL) {
		fprintf(stderr, "%s\n", "malloc() has failed.");
		exit(EXIT_FAILURE);
	}

	return ret;
}

void ewc(int code, const char* message, ...) {
	va_list args;
	va_start(args, message);
	if (code == EXIT_SUCCESS) {
		vfprintf(stdout, message, args);
		fputc('\n', stdout);
	}
	else {
		vfprintf(stderr, message, args);
		fputc('\n', stderr);
	}
	va_end(args);
	exit(code);
}

char* strrep(const char* string, const char* substr, const char* replacement) {
	char* tok = NULL;
	char* newstr = NULL;
	char* oldstr = NULL;
	if (substr == NULL || replacement == NULL)
		return strdup(string);

	newstr = strdup(string);
	while ((tok = strstr(newstr, substr))) {
		oldstr = newstr;
		newstr = emalloc(strlen(oldstr) - strlen(substr) + strlen(replacement) + 1);

		memcpy(newstr, oldstr, tok - oldstr);
		memcpy(newstr + (tok - oldstr), replacement, strlen(replacement));
		memcpy(newstr + (tok - oldstr) + strlen(replacement), tok + strlen(substr), strlen(oldstr) - strlen(substr) - (tok - oldstr));
		memset(newstr + strlen(oldstr) - strlen(substr) + strlen(replacement), 0, 1);

		free(oldstr);
	}

	return newstr;
}
