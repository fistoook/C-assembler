#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "intialize_data_struct.h"

#define MAX_LINE_LENGTH 80  /* Maximum length for a line of input */
#define INTIAL_INSTRUCT_CNT_SIZE 0  /* Initial size of instruction count */
#define INTIAL_DATA_CNT_SIZE 0  /* Initial size of data count */
#define ADDITIONAL_AMOUNT_OF_LABELS 5  /* Amount of labels to add when resizing */
#define INTIAL_AMOUNT_OF_EXT_ENT_LABELS 5  /* Initial size for external and entry labels */

/* Finds a word in a string starting from a given position */
char *find_word(const char *, int);

/* Checks if the length of a line is valid */
int check_line_lengths(const char *);

/* Checks if a label exists in the label table */
int label_exists(label_table *, const char *);

/* Inserts a new label into the label table */
int insert_label(label_table *, const char *, int, int, int, int, int, location *);

/* Handles operands in directives */
void handle_directive_operands(char *, int, int, int, FILE *, label_table *, char *, location *, int);

/* Adds machine code data to the given code_conv array */
int add_machine_code_data(code_conv **, int *, location *, const char *, const char *, label *, int);

/* Validates if the instruction is valid */
int is_valid_instr(char *, location *);

/* Parses an instruction and updates the instruction struct */
int parse_instruction(const char *, char *, instruction *, label_table *, location *, code_conv **, int *, int *, label *);

/* Finds the position in the string after a directive */
char *find_position_after_directive(char *, char *);

/* Updates label addresses in the label table */
void update_label_addresses(label_table *, int);

#endif
