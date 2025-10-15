#include "parser.h"

int find_opcode_index(const char *instruction_name) {
    /* Array of valid instruction names corresponding to the opcodes enum */
    const char *instructions[] = {
        "mov", "cmp", "add", "sub", "lea",
        "clr", "not", "inc", "dec", "jmp",
        "bne", "red", "prn", "jsr", "rts", "stop"
    };

    int num_instructions = sizeof(instructions) / sizeof(instructions[0]);
    int i;

    /* Search for the instruction name in the instructions array */
    for (i = 0; i < num_instructions; i++) {
        if (strcmp(instruction_name, instructions[i]) == 0) {
            return i;  /* Return the index if found */
        }
    }

    return -1;  /* Return -1 if the instruction name is not found */
}

int parse_two_operands(char *operands, instruction *instr, label_table *table, location *am_file, const char *instruction_name);
int parse_one_operand(char *operand, instruction *instr, label_table *table, location *am_file);

int parse_instruction(const char *instruction_name, char *operands, instruction *instr, label_table *table, location *am_file, code_conv **instructions, int *IC, int *cc_capacity, label *label) {
    int opcode_index, L;

    /* Step 1: Validate the instruction name */
    opcode_index = find_opcode_index(instruction_name);

    /* Initialize the instruction struct with the opcode */
    instr->opcode = (opcodes)opcode_index;

    /* Step 2: Parse the operands and calculate L (Length of the instruction in memory words) */
    if (strcmp(instruction_name, "mov") == 0 || strcmp(instruction_name, "cmp") == 0 || strcmp(instruction_name, "add") == 0 || strcmp(instruction_name, "sub") == 0 || strcmp(instruction_name, "lea") == 0) {
        /* For these instructions, we expect two operands */
        instr->operand_count = 2;
        L = parse_two_operands(operands, instr, table, am_file, instruction_name);
    } 
    else if (strcmp(instruction_name, "not") == 0 || strcmp(instruction_name, "clr") == 0 || strcmp(instruction_name, "inc") == 0 || strcmp(instruction_name, "dec") == 0 || strcmp(instruction_name, "jmp") == 0 || strcmp(instruction_name, "bne") == 0 || strcmp(instruction_name, "red") == 0 || strcmp(instruction_name, "prn") == 0 || strcmp(instruction_name, "jsr") == 0) {
        /* For these instructions, we expect one operand */
        instr->operand_count = 1;
        L = parse_one_operand(operands, instr, table, am_file);
    } 
    else if (strcmp(instruction_name, "rts") == 0 || strcmp(instruction_name, "stop") == 0) {
        /* For these instructions, no operands are expected */
        instr->operand_count = 0;
        instr->length = 1; /* Only one word is needed */
    } 
    else {
        PRINT_ERROR1(am_file->file_name, am_file->line, "Unexpected instruction format for: %s", instruction_name);
        return 0;
    }

    /* Step 3: Encode the binary representation for the first word of the instruction */
    encode_instruction_first_word(instr);

    /* Ensure there is enough space in the instructions array */
    if (*IC >= *cc_capacity) {
        code_conv *new_instructions;
        *cc_capacity *= 2;  /* Double the capacity to minimize realloc calls */
        new_instructions = realloc(*instructions, (*cc_capacity) * sizeof(code_conv));
        if (!new_instructions) {
            PRINT_ERROR(am_file->file_name, am_file->line, "Memory allocation failed");
            exit(1);
        }
        *instructions = new_instructions;
    }

    /* Store the encoded first word of the instruction */
    (*instructions)[*IC].binary_repres = instr->binary_repres;
    (*instructions)[*IC].label[0] = NULL; /* No label for the first word */
    (*instructions)[*IC].assembly_line = am_file->line;
    (*IC)++;  /* Increment IC after storing the instruction */

    /* Step 4: Encode and store the operands using the new encode_operands function */
    encode_operands(instr->operand_count > 0 ? &instr->operands[0] : NULL, 
                    instr->operand_count > 1 ? &instr->operands[1] : NULL, 
                    instructions, IC, cc_capacity, table, am_file, label);

    return L;
}


operand parse_operand(char *operand_str, label_table *table, location *am_file);
int calculate_instruction_length(operand *operands, int operand_count);

/* Parse and handle two operands for an instruction. */
int parse_two_operands(char *operands, instruction *instr, label_table *table, location *am_file, const char *instruction_name) {
    char *operand1 = NULL, *operand2 = NULL;
    int i, len = strlen(operands), comma_found = 0, L;

    /* Find and split at comma */
    for (i = 0; i < len; i++) {
        if (operands[i] == ',') {
            if (comma_found) {
                PRINT_ERROR1(am_file->file_name, am_file->line, "Multiple consecutive commas in '%s' instruction", instruction_name);
                return 0;
            }
            comma_found = 1;
            operands[i] = '\0';
            operand2 = operands + i + 1;
        } else if (!comma_found) {
            operand1 = operands;
        }
    }

    /* Check for valid operands */
    if (!comma_found || !operand1 || !operand2) {
        PRINT_ERROR1(am_file->file_name, am_file->line, "Illegal operand amount! Expected 2 operands for '%s' instruction", instruction_name);
        return 0;
    }

    /* Trim whitespace and parse operands */
    operand1 = trim_whitespace(operand1);
    operand2 = trim_whitespace(operand2);
    instr->operands[0] = parse_operand(operand1, table, am_file);
    instr->operands[1] = parse_operand(operand2, table, am_file);

    /* Calculate and return instruction length */
    instr->length = calculate_instruction_length(instr->operands, 2);
    L = instr->length;
    return L;
}

/* Parse and handle a single operand for an instruction. */
int parse_one_operand(char *operand_str, instruction *instr, label_table *table, location *am_file) {
    char *operand = NULL;
    int i, len = strlen(operand_str), L;

    /* Trim leading and trailing whitespace */
    operand_str = trim_whitespace(operand_str);

    /* Find the start of the operand, check for illegal commas */
    for (i = 0; i < len; i++) {
        if (operand_str[i] == ',') {
            PRINT_ERROR(am_file->file_name, am_file->line, "Illegal comma found in instruction");
            return 0;
        }
        if (!isspace(operand_str[i])) {
            if (operand) {
                break;
            }
            operand = operand_str + i;
        }
    }

    /* Check if operand was found */
    if (!operand) {
        PRINT_ERROR(am_file->file_name, am_file->line, "Illegal operand amount! Expected 1 operand for this instruction");
        return 0;
    }

    /* Trim whitespace and parse operand */
    operand = trim_whitespace(operand);
    instr->operands[0] = parse_operand(operand, table, am_file);

    /* Calculate and return instruction length */
    instr->length = calculate_instruction_length(instr->operands, 1);
    L = instr->length;

    return L;
}


#define FIRST_REG_INDEX 0
#define LAST_REG_INDEX 7

char *find_label(label_table *, const char *);
void print_regs(int);
void print_labels(label_table *);

operand parse_operand(char *operand_str, label_table *table, location *am_file) {
    operand opr;
    opr.is_label = 0;
    opr.value = 0;
    opr.is_register = 0;
    opr.register_index = -1;
    opr.type = 0;

    /* Check if operand is a number with # prefix */
    if (operand_str[0] == '#') {
        char *number_str = operand_str + 1;
        /* Check if the number is valid */
        if (!number_str)
            PRINT_ERROR(am_file -> file_name, am_file -> line, "Missing number");
        if (isdigit(number_str[0]) || (number_str[0] == '-' && isdigit(number_str[1])) || (number_str[0] == '+' && isdigit(number_str[1]))) {
            opr.value = atoi(number_str);
            opr.type = IMMEDIATE;
        }
        else
            PRINT_ERROR1(am_file -> file_name, am_file -> line, "Illegal number format '%s'", operand_str);
    }
    else if (operand_str[0] == '*') {
        if (operand_str[1] == 'r' && isdigit(operand_str[2]) && operand_str[3] == '\0') {
            int reg_num = operand_str[2] - '0';
            if (reg_num >= FIRST_REG_INDEX && reg_num <= LAST_REG_INDEX) {
                opr.is_register = 1;
                opr.register_index = reg_num;
                opr.type = INDIRECT_REG;
            }
            else {
                PRINT_ERROR1(am_file -> file_name, am_file -> line, "Illegal register '%s', the legal registers are:", operand_str);
                print_regs(FIRST_REG_INDEX);
            }
        }
        else {
            PRINT_ERROR1(am_file -> file_name, am_file -> line, "Illegal pointer format '%s'", operand_str);
        }
    }
    /* Check if operand is a register */
    else if (operand_str[0] == 'r' && isdigit(operand_str[1]) && operand_str[2] == '\0') {
        int reg_num = operand_str[1] - '0';
        if (reg_num >= FIRST_REG_INDEX && reg_num <= LAST_REG_INDEX) {
            opr.is_register = 1;
            opr.register_index = reg_num;
            opr.type = DIRECT_REG;
        }
        else {
            PRINT_ERROR1(am_file -> file_name, am_file -> line, "Illegal register '%s', the legal registers are:", operand_str);
            print_regs(FIRST_REG_INDEX);
        }
    }
    /* Check if operand is a label */
    else if (isalpha(operand_str[0])) {
    	opr.type = DIRECT;
    	opr.is_label = 1;
    	if (!(strlen(operand_str) < MAX_LABEL_LENGTH)) 
        	PRINT_ERROR1(am_file->file_name, am_file->line, "Label name '%s' is too long. Max length is 31", operand_str); 
    }
    /* Operand is illegal */
    else {
        PRINT_ERROR1(am_file -> file_name, am_file -> line, "Illegal operand '%s'", operand_str);
    }

    return opr;
}

char *find_label_name(label_table *table, const char *label_name) {
    int i;

    /* Iterate through the label table to find the label */
    for (i = 0; i < table -> count; i++) {
        if (strcmp(table -> labels[i].name, label_name) == 0) {
            return table -> labels[i].name;  /* Return the name of the found label */
        }
    }

    return NULL;  /* Return NULL if the label is not found */
}

void print_regs(int reg_num) {
    if (reg_num > 7) {
        return;
    }

    /* Print current register */
    printf("\033[0;32mr%d\033[0m", reg_num);

    /* If it's not the last register, print a comma */
    if (reg_num < 7) {
        printf(", ");
    }

    /* Recursive call for the next register */
    print_regs(reg_num + 1);
}

/* Function to print labels recursively */
void print_labels_recursive(label *labels, int index, int total) {
    /* Base case: if the index is out of bounds, return */
    if (index >= total) {
        return;
    }

    /* Print the label's name */
    printf("\033[0;32m%s\033[0m", labels[index].name);

    /* Print a comma if it's not the last label */
    if (index < total - 1) {
        printf(", ");
    }
    /* Recursive call to print the next label */
    print_labels_recursive(labels, index + 1, total);
}



/* Function to start the recursive printing */
void print_labels(label_table *table) {
    if (table -> count > 0) {
        print_labels_recursive(table -> labels, 0, table -> count);
        printf("\n"); /* New line after all labels are printed */
    }
}

int calculate_instruction_length(operand *operands, int operand_count) {
    int length_in_words = 1;  /* Start with 1 for the opcode word */
    int i;
    int has_register_pair = 0;

    for (i = 0; i < operand_count; i++) {
        if (operands[i].type == IMMEDIATE || operands[i].type == DIRECT) {
            /* Immediate or direct operand adds one word */
            length_in_words++;
        }
        else if (operands[i].is_register || operands[i].type == DIRECT_REG || operands[i].type == INDIRECT_REG) {
            /* Check if there's already a register operand */
            if (has_register_pair) {
                has_register_pair = 0;
            }
            else {
                /* Mark that we've encountered a register operand */
                has_register_pair = 1;
                length_in_words++;
            }
        }
    }

    return length_in_words;
}

