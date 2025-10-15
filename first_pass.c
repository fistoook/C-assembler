#include "first_pass.h"
#include "globals.h"
#include "util.h"
#include "second_pass.h"

int execute_first_pass(char *am_file_name) {
    /* step 1: define and intialize the needed variables */
    int DC = INTIAL_DATA_CNT_SIZE, IC = INTIAL_INSTRUCT_CNT_SIZE; /* define and initialize the istruction and data counters */
    FILE *fp;
    char line[MAX_LINE_LENGTH + 2];
    int line_counter = 0;
    label_table table, extern_entry;
    location *am_file;
    code_conv *data;
    code_conv *instructions;
    int cc_capacity = INITIAL_CC_CAPACITY;
    instruction *instr;
	
    fp = fopen(am_file_name, "r"); /* open the am file (the file after macro extension) for reading */
    if (!fp) {
        PRINT_ERROR1(am_file_name, line_counter, "Unable to open file '%s'.", am_file_name);
        return 0;
    }

    initialize_location(&am_file, am_file_name);
    initialize_label_table(&table);
    initialize_label_table(&extern_entry);
    initialize_code_conv(&data);
    initialize_code_conv(&instructions);
	
    /* step 2: read the next line from the file */
    while (fgets(line, MAX_LINE_LENGTH, fp)) {
        char *first_word;
        int first_word_len, label_flag = 0;
		char *instruction_name;
        char *operands, *after_directive;
        label *curr_lbl = NULL;
        
        line_counter++;
        am_file -> line++;
		
		initialize_instruction(&instr);
		
        first_word = find_word(line, 0);
        first_word_len = strlen(first_word) - 1;

        /* step 3: check if the first word is a label */
        if (first_word[first_word_len] == ':') {
            first_word[first_word_len] = '\0';
            curr_lbl = (label *) malloc(sizeof(label));
            if (!curr_lbl) {
            	printf("MEMORY_ALLOCATION_FAILED");
            	fclose(fp);
                free_label_table(&table);
                free_label_table(&extern_entry);
                free(first_word);
            }
            strcpy(curr_lbl -> name, first_word);
            label_flag = 1; /* step 4: turn on label definiton falg */
        }
        if (label_flag) { /* inside label definition */
            char *directive = find_word(line, first_word_len + 2); /* find the directive */
            char *operands, *instruction_name;
			char *after_directive;
            if (!directive) {
                PRINT_ERROR1(am_file -> file_name, am_file -> line, "Missing directive after label '%s'.", first_word);
                fclose(fp);
                free_label_table(&table);
                free_label_table(&extern_entry);
                free(curr_lbl);
                free(first_word);
                return 0;
            }
			
			after_directive = find_position_after_directive(line, directive);
            if ((!after_directive || only_space_remain(after_directive)) && strcmp(directive, "stop") != 0 && strcmp (directive, "rts") != 0) {
                PRINT_ERROR1(am_file -> file_name, am_file -> line, "Missing parameters after directive '%s' in label.", directive);
                fclose(fp);
                free_label_table(&table);
                free_label_table(&extern_entry);
                free(curr_lbl);
                free(first_word);
                free(directive);
                return 0;
            }

            operands = after_directive;
            /* step 5: check if the directive is .data or .string */
            if (strcmp(directive, ".data") == 0 || strcmp(directive, ".string") == 0) {
                int label_index;
                label *current_label;
                /* step 6: add the label to the table with appropraite data */
                if (!insert_label(&table, first_word, DC, line_counter, 1, 0,0, am_file)) {
                    fclose(fp);
                    free_label_table(&table);
                    free_label_table(&extern_entry);
                    free(curr_lbl);
                    free(first_word);
                	free(directive);
                    return 0;
                }

                label_index = table.count - 1;
                current_label = &table.labels[label_index];
                /* step 7: Identify the data type, encode it in memory, and refine DC accordingly */
                if (!add_machine_code_data(&data, &DC, am_file, directive, operands, current_label, IC)) {
                    fclose(fp);
                    free_label_table(&table);
                    free_label_table(&extern_entry);
                    free(curr_lbl);
                    free(first_word);
                	free(directive);
                    return 0;
                }
                /* step 7 complete. go back to step 2 */
                free(first_word);
                free(curr_lbl);
                free(directive);
                continue;
            }
            /* step 8: check if the directive is .extern or .entry */
            else if (strcmp(directive, ".entry") == 0 || strcmp(directive, ".extern") == 0) {
                int is_extern, is_entry;
                is_extern = (strcmp(directive, ".extern") == 0);
                is_entry = (strcmp(directive, ".entry") == 0);

                /* print a warning because the label has no effect */
                PRINT_WARNING1(am_file_name, line_counter, "label '%s' has no effect", first_word);

                /* step 9: */
                handle_directive_operands(operands, line_counter, is_extern, is_entry, fp, &extern_entry, line, am_file, DC);
                free(first_word);
                free(curr_lbl);
                free(directive);
                continue;
            }
            /* step 10: Insert the label with the code property */
            if (!insert_label(&table, first_word, IC + 100, line_counter, 0, 0, 0, am_file)) {
                fclose(fp);
                free_label_table(&table);
                free_label_table(&extern_entry);
                free(first_word);
                free(curr_lbl);
                free(directive);
                return 0;

            }

            /* step 11: we will start to parse and process the instruction */
            instruction_name = directive;
            if (!is_valid_instr(instruction_name, am_file)) {
                fclose(fp);
                free_label_table(&table);
                free_label_table(&extern_entry);
                free(first_word);
                free(curr_lbl);
                free(instruction_name);
                return 0;
            }

            /* step 12: parse the instruction, calculate L, encode the first word */
            parse_instruction(instruction_name, operands, instr, &table, am_file, &instructions, &IC, &cc_capacity, curr_lbl);
		

            free(first_word);
            free(curr_lbl);
            free(instruction_name);
            continue;
        }
        /* not a label, go to step 5 */
        /* step 5: check if the directive is .data or .string */
        after_directive = find_position_after_directive(line, first_word);
        operands = after_directive;

        if (strcmp(first_word, ".data") == 0 || strcmp(first_word, ".string") == 0) {
            int label_index = table.count - 1;
            label *current_label = &table.labels[label_index];
            /* step 7: Identify the data type, encode it in memory, and refine DC accordingly */
            if (!add_machine_code_data(&data, &DC, am_file, first_word, operands, current_label, IC)) {
                fclose(fp);
                free_label_table(&table);
                free(first_word);
                return 0;
            }
            /* step 7 complete. go back to step 2 */
            free(first_word);
            continue;
        }
        /* step 8: check if the directive is .extern or .entry */
        else if (strcmp(first_word, ".entry") == 0 || strcmp(first_word, ".extern") == 0) {
            int is_extern, is_entry;
            is_extern = (strcmp(first_word, ".extern") == 0);
            is_entry = (strcmp(first_word, ".entry") == 0);

            handle_directive_operands(operands, line_counter, is_extern, is_entry, fp, &extern_entry, line, am_file, DC);
            free(first_word);
            continue;
        }
        /* step 11: we will start to parse and process the instruction */
        instruction_name = first_word;
        if (!is_valid_instr(instruction_name, am_file)) {
            fclose(fp);
            free_label_table(&table);
            free(first_word);
            return 0;
        }

        /* step 12: parse the instruction, calculate L, encode the first word */
        parse_instruction(instruction_name, operands, instr, &table, am_file, &instructions, &IC, &cc_capacity, curr_lbl);

        free(first_word);
        continue;
    }
    
    /* test_encoding_output(data, DC, instructions, IC); */
    update_label_addresses(&table, IC);
    execute_second_pass(fp, instructions, data, table, am_file, &cc_capacity, DC, extern_entry);
    free_label_table(&table);
    free_label_table(&extern_entry);
    return 1;
}

int label_exists(label_table *table, const char *label_name) {
    int i;
    for (i = 0; i < table -> count; i++)
        if (strcmp(table -> labels[i].name, label_name) == 0)
            return 1;
    return 0;
}

/* Insert a new label into the label table. */
int insert_label(label_table *table, const char *label_name, int address, int assembly_line, int is_data, int is_external, int is_entry, location *am_file) {
    label *label_to_insert;

    /* Check if the label already exists */
    if (label_exists(table, label_name)) {
        PRINT_ERROR1(am_file->file_name, am_file->line, "Label '%s' already exists.", label_name);
        return 0;
    }

    /* Reallocate space if needed */
    if (table->count >= table->capacity) {
        label *new_labels;
        table->capacity += ADDITIONAL_AMOUNT_OF_LABELS;
        new_labels = (label *) realloc(table->labels, sizeof(label) * table->capacity);
        if (!new_labels) {
            printf("MEMORY ALLOCATION FAILED\n");
            exit(1);
        }
        table->labels = new_labels;
    }

    /* Initialize and add the new label */
    label_to_insert = &table->labels[table->count++];
    strcpy(label_to_insert->name, label_name);
    label_to_insert->address = address;
    label_to_insert->assembly_line = assembly_line;
    label_to_insert->is_data = is_data;
    label_to_insert->is_external = is_external;
    label_to_insert->is_entry = is_entry;

    return 1;
}

void handle_directive_operands(char *operands, int line_counter, int is_extern, int is_entry, FILE *fp, label_table *table, char *line, location *am_file, int DC) {
    while (*operands != '\0') {
        char *symbol_start;
        while (isspace(*operands)) operands++; /* skip whitespaces */

        symbol_start = operands; /* new operand */
        while (*operands != ',' && *operands != '\0' && !isspace(*operands)) operands++; /* find end of operand */
		
		remove_leading_whitespace(operands);
        if (*operands == ',' || *operands == '\0') {
            char symbol[MAX_LABEL_LENGTH + 1];
            int symbol_len = operands - symbol_start; /* calculate symbol len */
            char *end;

            strncpy(symbol, symbol_start, symbol_len); /* copy operand */
            symbol[symbol_len] = '\0';

            /* Trim trailing whitespaces from the symbol */
            end = symbol + symbol_len - 1;
            while (end > symbol && isspace(*end)) end--;
            *(end + 1) = '\0';

            /* Insert the label */
            if (!insert_label(table, symbol, DC, line_counter, 0, is_extern, is_entry, am_file)) {
                fclose(fp);
                free_label_table(table);
                exit(0);
            }

            /* If the operand is followed by a comma, proceed to the next operand */
            if (*operands == ',') {
                operands++;
                while (isspace(*operands)) operands++; /* skip whitespaces after comma */

                /* Check for multiple consecutive commas */
                if (*operands == ',') {
                    PRINT_ERROR(am_file->file_name, am_file->line, "Multiple consecutive commas.");
                    fclose(fp);
                    free_label_table(table);
                    exit(0);
                }

                /* Check for a missing operand after the comma */
                if (*operands == '\0') {
                    PRINT_ERROR(am_file->file_name, am_file->line, "Missing operand.");
                    fclose(fp);
                    free_label_table(table);
                    exit(0);
                }
            }
            else if (*operands == '\0') {
                break; /* End of string, exit the loop */
            }
        } else {
            PRINT_ERROR(am_file->file_name, am_file->line, "Missing comma.");
            fclose(fp);
            free_label_table(table);
            exit(0);
        }
    }
}

void print_all_instructions(const char *opcode_names[], int num_instructions) {
    int i;

    printf("\033[0;32m"); /* Set text color to green */
    printf("List of all instructions: ");
    for (i = 0; i < num_instructions; i++) {
        printf("%s", opcode_names[i]);
        if (i < num_instructions - 1)
            printf(", ");
    }
    printf("\033[0m\n"); /* Reset text color */
}

/* Check if the provided instruction name is valid. */
int is_valid_instr(char *instr_name, location *am_file) {
    /* List of valid instructions */
    const char *valid_instructions[] = {
        "mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec",
        "jmp", "bne", "red", "prn", "jsr", "rts", "stop"
    };

    int instruction_count = sizeof(valid_instructions) / sizeof(valid_instructions[0]);
    int i;

    /* Compare instruction name with valid instructions */
    for (i = 0; i < instruction_count; i++) {
        if (strcmp(instr_name, valid_instructions[i]) == 0)
            return 1; /* Instruction is valid */
    }

    /* Print error if instruction is invalid */
    PRINT_ERROR1(am_file->file_name, am_file->line, "Invalid instruction name: '%s' the valid instructions are: \n", instr_name);
    print_all_instructions(valid_instructions, instruction_count);
    return 0; /* Instruction is not valid */
}

char *find_position_after_directive(char *line, char *directive) {
    const char *directive_pos = strstr(line, directive);
    
    if (directive_pos != NULL) {
        /* Move the pointer to the end of the directive */
        char * position_after_directive = (char *)(directive_pos + strlen(directive));
        /* Skip any whitespace after the directive */
        while (*position_after_directive == ' ' || *position_after_directive == '\t') {
            position_after_directive++;
        }
        return position_after_directive;
    }
    
    return NULL;  /* Directive not found */
}

void update_label_addresses(label_table *table, int IC) {
    int i;
    for (i = 0; i < table->count; i++) {
        if (table->labels[i].is_data) {
            table->labels[i].address += IC + 100;
        }
    }
}

