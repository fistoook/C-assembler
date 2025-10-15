#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "intialize_data_struct.h"
#include "globals.h"
#include "util.h"

#define IMMEDIATE 1
#define DIRECT 2
#define INDIRECT_REG 3
#define DIRECT_REG 4

void encode_instruction_first_word(instruction *instr);
void encode_operands(operand *op1, operand *op2, code_conv **instructions, int *IC, int *cc_capacity, label_table *table, location *am_file, label *lbl);

#endif

