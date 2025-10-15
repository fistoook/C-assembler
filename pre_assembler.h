#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_LINE_LENGTH 80

/* Structure representing a macro with its name, lines of code, line count, and a pointer to the next macro in a linked list. */
typedef struct Macro {
    char *name;            /* The name of the macro */
    char **lines;          /* Array of lines of code in the macro */
    int line_count;        /* Number of lines in the macro */
    int capacity;          /* Capacity of the lines array */
    struct Macro *next;    /* Pointer to the next macro in the list */
} Macro;

/* Structure representing a table of macros, which is essentially a linked list of Macro structures. */
typedef struct {
    Macro *head;           /* Head of the linked list of macros */
} MacroTable;

/* Array of reserved macro names that are not allowed as macro names. */
char *illegal_macro_names[] = {
    "mov",
    "cmp",
    "add",
    "sub",
    "lea",
    "not",
    "clr",
    "inc",
    "dec",
    "jmp",
    "bne",
    "red",
    "prn",
    "jsr",
    "rts",
    "stop",
    ".data",
    ".string",
    ".extern",
    ".entry"
};

/* Create and open an output file with the given filename.
   @param filename: The name of the file to create.
   @return: A FILE pointer to the created file, or NULL on failure. */
FILE *create_output_file(const char *filename);

/* Check if a macro name is legal or not, based on a predefined list of illegal macro names.
   @param macro_name: The name of the macro to check.
   @return: 1 if the macro name is legal, 0 otherwise. */
int is_legal_macro(const char *macro_name);

/* Free the memory allocated for a macro table, including all macros and their lines.
   @param table: A pointer to the MacroTable to free. */
void free_macro_table(MacroTable *table);

#endif
