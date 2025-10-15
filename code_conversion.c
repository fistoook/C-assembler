#include "code_conversion.h"
#include "globals.h"
#include "parser.h"

/* Function to encode parsed data into memory */
int encode_data_to_memory(code_conv **data, int *DC, location *am_file, const int *values, int count, label *label, int IC) {
    int i;
    for (i = 0; i < count; i++) {
        /* Reallocate memory for data array to hold additional entry */
        *data = realloc(*data, (*DC + 1) * sizeof(code_conv));
        if (!*data) {
            printf("MEMORY REALLOCATION FAILED\n");
            return 0;  /* Return 0 if memory reallocation fails */
        }
        /* Set the binary representation of the data */
        (*data)[*DC].binary_repres = (unsigned short)values[i];
        /* Set the assembly line from the location information */
        (*data)[*DC].assembly_line = am_file -> line;
        (*DC)++;  /* Increment data count */
    }
    return 1;  /* Return 1 to indicate success */
}

/* Function to handle .data and .string directives */
int add_machine_code_data(code_conv **data, int *DC, location *am_file, const char *directive, const char *operands, label *label, int IC) {
    if (strcmp(directive, ".data") == 0) {
        int count = 0;
        int *values = parse_operands(operands, am_file, &count);
        if (!values) {
            PRINT_ERROR(am_file -> file_name, am_file -> line, "Failed to parse operands for .data directive.");
            return 0;  /* Return 0 if parsing operands fails */
        }

        /* Encode the parsed data into memory */
        if (!encode_data_to_memory(data, DC, am_file, values, count, label, IC)) {
            free(values);  /* Free memory if encoding fails */
            return 0;
        }
        free(values);  /* Free memory after successful encoding */
    } else if (strcmp(directive, ".string") == 0) {
        /* Locate the starting and ending quotes */
        const char *start_quote = strchr(operands, '"');
        const char *end_quote = strrchr(operands, '"');
        const char *p;
        if (start_quote && end_quote) {
            if (start_quote != end_quote) {
                /* Move past the starting quote */
                start_quote++;
                /* Store each character in the string, including the null terminator */
                for (p = start_quote; p < end_quote; p++) {
                    *data = realloc(*data, (*DC + 1) * sizeof(code_conv));
                    if (!*data) {
                        printf("MEMORY REALLOCATION FAILED\n");
                        return 0;  /* Return 0 if memory reallocation fails */
                    }
                    (*data)[*DC].binary_repres = (unsigned short)(*p);
                    (*data)[*DC].label[0] = NULL;  /* No label associated with this data */
                    (*data)[*DC].assembly_line = am_file -> line;
                    (*DC)++;
                }
                /* Add null terminator to the data */
                *data = realloc(*data, (*DC + 1) * sizeof(code_conv));
                if (!*data) {
                    printf("MEMORY REALLOCATION FAILED\n");
                    return 0;  /* Return 0 if memory reallocation fails */
                }
                (*data)[*DC].binary_repres = 0;  /* Null terminator */
                (*data)[*DC].label[0] = NULL;  /* No label associated with this data */
                (*data)[*DC].assembly_line = am_file -> line;
                (*DC)++;
            }
        } else {
            PRINT_ERROR(am_file -> file_name, am_file -> line, "Invalid string format.");
            return 0;  /* Return 0 if the string format is invalid */
        }
    }
    return 1;  /* Return 1 to indicate success */
}

int *parse_operands(const char *operands, location *am_file, int *count) {
    const char *ptr = operands;
    int values_size = INITIAL_VAL_BUF_SIZE, current_line = am_file -> line;
    int *values = (int *) malloc(values_size * sizeof(int));
    if (!values) {
        printf("Memory allocation failed\n");
        return 0;
    }

    while (*ptr) {
        char *number_buffer;
        int number_index, current_size;
        /* Skip whitespace */
        while (isspace(*ptr)) ptr++;

        /* Handle empty operand */
        if (*ptr == '\0') {
            break;
        }

        number_buffer = (char *) malloc(INITIAL_VAL_BUF_SIZE);
        if (!number_buffer) {
            printf("MEMORY ALLOCATION FAILED\n");
            free(values);
            return 0;
        }

        current_size = INITIAL_VAL_BUF_SIZE;
        number_index = 0;

        /* Parse the number */
        while (isdigit(*ptr) || *ptr == '-' || *ptr == '+') {
            if (number_index >= current_size - 1) {
                current_size += ADDITIONAL_VAL_BUF_SIZE;
                number_buffer = (char *) realloc(number_buffer, current_size);
                if (!number_buffer) {
                    printf("MEMORY ALLOCATION FAILED\n");
                    free(values);
                    return 0;
                }
            }
            number_buffer[number_index++] = *ptr++;
        }
        number_buffer[number_index] = '\0';

        if (number_index > 0) {
            /* Check if values array needs to grow */
            if (*count >= values_size) {
                values_size += ADDITIONAL_VAL_BUF_SIZE;
                values = (int *) realloc(values, values_size * sizeof(int));
                if (!values) {
                    printf("MEMORY ALLOCATION FAILED\n");
                    free(number_buffer);
                    return 0;
                }
            }
            values[(*count)++] = atoi(number_buffer);
        }
        else {
            PRINT_ERROR1(am_file -> file_name, current_line, "Invalid operand '%s' not an int", number_buffer);
            free(number_buffer);
            free(values);
            return 0;
        }

        free(number_buffer);

        /* Skip whitespace after the number */
        while (isspace(*ptr)) ptr++;

        /* Check for multiple commas */
        if (*ptr == ',') {
            ptr++;
            /* Skip whitespace after the comma */
            while (isspace(*ptr)) ptr++;
            if (*ptr == ',') {
                PRINT_ERROR(am_file -> file_name, current_line, "Multiple consetive commas");
                free(values);
                return 0;
            }
        }
        else if (*ptr != '\0') {
            PRINT_ERROR(am_file -> file_name, current_line, "Expected comma or end of line");
            free(values);
            return 0;
        }
    }

    /* Check for trailing comma */
    if (*count > 0 && *(ptr - 1) == ',') {
        PRINT_ERROR(am_file -> file_name, current_line, "Trailing comma");
        free(values);
        return 0;
    }

    return values;
}

void encode_instruction_first_word(instruction *instr) {
    unsigned short first_word = 0;  /* Initialize the first word to 0 */

    first_word |= (instr -> opcode << 11);  /* Encode the opcode into bits 14-11 */

    if (instr->operand_count > 1) {
        /* Process the first operand if there are more than one operand */
        if (instr -> operands[0].is_label) {
            first_word |= (1 << 8);  /* Set bit 8 for label operand */
        }
        else if (instr -> operands[0].is_register) {
            /* Set bits 9 or 10 based on the register type */
            if (instr -> operands[0].type == INDIRECT_REG)
                first_word |= (1 << 9);  /* Set bit 9 for indirect register */
            else
                first_word |= (1 << 10);  /* Set bit 10 for direct register */
        }
        else 
            first_word |= (1 << 7);  /* Set bit 7 for immediate or direct value */
    }
    
    if (instr->operand_count > 0) {
        /* Process the last operand */
        if (instr -> operands[instr -> operand_count - 1].is_label) {
            first_word |= (1 << 4);  /* Set bit 4 for label operand */
        }
        else if (instr -> operands[instr -> operand_count - 1].is_register) {
            /* Set bits 5 or 6 based on the register type */
            if (instr -> operands[instr -> operand_count - 1].type == INDIRECT_REG)
                first_word |= (1 << 5);  /* Set bit 5 for indirect register */
            else
                first_word |= (1 << 6);  /* Set bit 6 for direct register */
        }
        else 
            first_word |= (1 << 3);  /* Set bit 3 for immediate or direct value */
    }

    first_word |= (1 << 2);  /* Set bit 2 to 1 (A,R,E field) */

    instr -> binary_repres = first_word;  /* Store the encoded first word in the instruction */
}

label *find_label(label_table *table, const char *label_name) {
	int i;
    for (i = 0; i < table->count; i++) {
        if (strcmp(table->labels[i].name, label_name) == 0) {
            return &table->labels[i];  /* Return pointer to the found label */
        }
    }
    return NULL;  /* Return NULL if the label is not found */
}


void encode_operands(operand *op1, operand *op2, code_conv **instructions, int *IC, int *cc_capacity, label_table *table, location *am_file, label *lbl) {
    unsigned short operand_word = 0;
    int i;
    int is_destanation = 0;
    operand *ops[2];

    /* Assign the operands to the array */
    ops[0] = op1;
    ops[1] = op2;
	
    /* Handle special case when both operands are registers */
    if (op1 && op2 && (op1->is_register && op2->is_register)) {
        
        /* Encode source register in bits 6-8 */
        operand_word |= (op1->register_index & 0x7) << 6;

        /* Encode destination register in bits 3-5 */
        operand_word |= (op2->register_index & 0x7) << 3;

        /* Set A,R,E field to A=1, R=0, E=0 */
        operand_word |= (1 << 2); 

        /* Allocate space for the new instruction word */
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

        /* Store the encoded word in the instructions array */
        (*instructions)[*IC].binary_repres = operand_word;
        if (lbl) {
    		(*instructions)[*IC].label[0] = my_strdup(lbl->name);
    		if ((*instructions)[*IC].label[0] == NULL) {
        		PRINT_ERROR(am_file->file_name, am_file->line, "Memory allocation failed");
        		exit(1);
    		}
		} else {
    		(*instructions)[*IC].label[0] = NULL;
		}
        (*instructions)[*IC].assembly_line = am_file->line;
        (*IC)++;  /* Increment IC after storing the instruction */

        return;  /* Return early since both operands were handled in one word */
    }

    /* Handle operands individually if they are not both registers */
    for (i = 0; i < 2; i++) {
        operand *op = ops[i];
		
        if (!op) continue;
		
		if (op && !ops[1])
			is_destanation = 1;
			
        operand_word = 0; /* Reset operand_word for each operand */

        switch (op->type) {
            case IMMEDIATE:
                operand_word |= (op->value & 0xFFF) << 3; /* 12-bit 2's complement value */
                operand_word |= (1 << 2); /* A,R,E field: A=1 */
                is_destanation = 1;
                break;

            case DIRECT:
                /* Placeholder for direct address handling (handled in the second pass) */
                is_destanation = 1;
                break;

            case INDIRECT_REG:
            	if (is_destanation) 
            		operand_word |= (op->register_index & 0x7) << 3;
            	else
                	operand_word |= (op->register_index & 0x7) << 6; /* Register index in bits 6-8 */
                operand_word |= (1 << 2); /* A,R,E field: A=1 */
                is_destanation = 1;
                break;

            case DIRECT_REG:
            	if (is_destanation) 
                	operand_word |= (op->register_index & 0x7) << 3; 
                else
                	operand_word |= (op->register_index & 0x7) << 6; 
                operand_word |= (1 << 2); /* A,R,E field: A=1 */
                is_destanation = 1;
                break;

            default:
                PRINT_ERROR(am_file->file_name, am_file->line, "Illegal operand type.");
                return;
        }

        /* Allocate space for the new instruction word */
        if (*IC >= *cc_capacity) {
            code_conv *new_instructions;
            *cc_capacity *= 2;  /* Double the capacity to minimize realloc calls */
            new_instructions = realloc(*instructions, (*cc_capacity) * sizeof(code_conv));
            if (!new_instructions) {
                PRINT_ERROR(am_file->file_name, am_file->line, "Memory allocation failed");
                exit(1);
            }
           
        }

        /* Store the encoded word in the instructions array */
        (*instructions)[*IC].binary_repres = operand_word;
        if (lbl) {
    		(*instructions)[*IC].label[0] = my_strdup(lbl->name);
    		if ((*instructions)[*IC].label[0] == NULL) {
        		PRINT_ERROR(am_file->file_name, am_file->line, "Memory allocation failed");
        		exit(1);
    		}
		} else {
    		(*instructions)[*IC].label[0] = NULL;
		}
        (*instructions)[*IC].assembly_line = am_file->line;
        (*IC)++;  /* Increment IC after storing the instruction */
    }
}

