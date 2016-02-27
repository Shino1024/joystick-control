#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void* emalloc(size_t size);
int ewc(int code, const char* message, ...);
char* strrep(const char* string, const char* substr, const char* replacement);

#endif