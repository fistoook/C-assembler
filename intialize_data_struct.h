#ifndef INTIALIZE_DATA_STRUCT_H
#define INTIALIZE_DATA_STRUCT_H

#define MAX_LABEL_LENGTH 31
#define INTIAL_AMOUNT_OF_LABELS 30
#define MAX_OPERANDS 2
#define INITIAL_CC_CAPACITY 5

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {
    mov,
    cmp,
    add,
    sub,
    lea,
    clr,
    not,
    inc,
    dec,
    jmp,
    bne,
    red,
    prn,
    jsr,
    rts,
    stop,
    INVALID_OPCODE = -1
} opcodes;

typedef struct {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
    int assembly_line;
    int is_data;
    int is_external;
    int is_entry;
} label;

typedef struct {
    label *labels;
    int count;
    int capacity;
} label_table;

typedef struct {
    unsigned short binary_repres;
    char *label[INTIAL_AMOUNT_OF_LABELS];
    int assembly_line;
} code_conv;

typedef struct {
    char *file_name;
    int line;
} location;

typedef struct {
    int type;            /* Type of the operand (e.g., immediate, direct, register) */
    int value;           /* The value of the operand (for immediate and register types) */
    int is_label;        /* Flag to indicate if the operand is a label */
    int is_register;     /* Flag to indicate if the operand is a register */
    int register_index;  /* Index of the register if the operand is a register */
} operand;

typedef struct {
    opcodes opcode;            /* The opcode for the instruction */
    int operand_count;         /* The number of operands */
    int length;                /* The length of the instruction in memory words */
    unsigned short binary_repres;
    operand operands[MAX_OPERANDS]; 
} instruction;

typedef struct {
    label *labels;  
    int count;      
    int capacity;   
} external_label_table;

void initialize_label_table(label_table *);
void free_label_table(label_table *);
void initialize_location(location **, char *);
void initialize_code_conv(code_conv **);
void free_code_conv(code_conv *);
void initialize_instruction(instruction **);
void free_instruction(instruction *);
void initialize_external_label_table(external_label_table *externals);

#endif
