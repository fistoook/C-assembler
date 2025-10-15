#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int execute_first_pass(char *am_file_name);
FILE *macro_extender(const char *source_file_name);

#endif

