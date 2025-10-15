#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdarg.h>

void print_error(const char *file, int line, const char *format, ...);
void print_warning(const char *file, int line, const char *format, ...);

#define PRINT_ERROR(file, line, format) print_error(file, line, format)
#define PRINT_ERROR1(file, line, format, arg1) print_error(file, line, format, arg1)

#define PRINT_WARNING(file, line, format) print_warning(file, line, format)
#define PRINT_WARNING1(file, line, format, arg1) print_warning(file, line, format, arg1)

#endif
