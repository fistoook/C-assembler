#include "first_pass.h"
#include "intialize_data_struct.h"
#include "parser.h"
#include "util.h"
#include "globals.h"

/* Find a label in the label table by its name */
label *find_label(label_table *table, const char *label_name);

/* Extract labels from the instruction's directive and operands and return them as an array of strings.
   Also set the number of labels found through the label_count pointer. */
char** extract_labels_from_instruction(const char *directive, const char *operands, int *label_count);

/* Parse operands to identify labels and handle them according to whether they are internal or external.
   Updates the instruction encoding and external label table as necessary. */
void parse_operands_for_labels(const char *operands, label_table *table, label_table *extern_entry, code_conv *instructions, int *IC, location *am_file, external_label_table *externals);

/* Find the index of an opcode in the opcode list based on the instruction name. */
int find_opcode_index(const char *instruction_name);

/* Create output files for object code, entry labels, and external labels based on the provided filename and tables. */
void create_output_files(const char *filename_with_ext, code_conv *instructions, int IC, code_conv *data, int DC, label_table *labels, external_label_table *externals, FILE *am_file, label_table *extern_entry);

void execute_second_pass(FILE *source_file, code_conv *instructions, code_conv *data, label_table labels, location *am_file, int *cc_capacity, int DC, label_table extern_entry) {
    char line[MAX_LINE_LENGTH];
    int IC = 0, errors_found = 0;
    external_label_table externals;
        
    /* Initialize line counter in the location structure */
    am_file->line = 0;
    
    /* Rewind the source file to the beginning */
    rewind(source_file);

    /* Initialize the external label table */
    initialize_external_label_table(&externals);

    /* Process each line from the source file */
    while (fgets(line, MAX_LINE_LENGTH, source_file)) {
        char *first_word;
        char *directive;
        char *operands;
        int first_word_len;
		
        /* Increment the line counter */
        am_file->line++;
        
        /* Find the first word in the line */
        first_word = find_word(line, 0);
        if (!first_word) { 
            continue;
        }
        first_word_len = strlen(first_word) - 1;

        /* If the first word ends with a colon, it's a label; adjust the directive accordingly */
        if (first_word[first_word_len] == ':') {
            first_word[first_word_len] = '\0';
            directive = find_word(line, first_word_len + 2);
        } else {
            directive = first_word;
        }

        /* Skip lines with directives (.data, .string, .extern, .entry) */
        if (strcmp(directive, ".data") == 0 || strcmp(directive, ".string") == 0 || 
            strcmp(directive, ".extern") == 0 || strcmp(directive, ".entry") == 0) {
            continue;
        }
        
        /* Find operands after the directive */
        operands = find_position_after_directive(line, directive);
        if (!operands || only_space_remain(operands)) {
            IC++;  
            continue;
        }

        /* Increment instruction counter and parse operands for labels */
        IC++;
        parse_operands_for_labels(operands, &labels, &extern_entry, instructions, &IC, am_file, &externals);
    }

    /* Print error message if errors were found; otherwise, create output files */
    if (errors_found) {
        printf("Errors were found during the second pass. Assembly process aborted.\n");
    } else {
        create_output_files(am_file->file_name, instructions, IC, data, DC, &labels, &externals, source_file, &extern_entry);
        printf("\nSecond pass completed successfully.\n");
    }

    /* Free memory allocated for external labels */
    free(externals.labels);
}

#define A_BIT 4  /* Bit mask for the A field in the encoding */
#define R_BIT 2  /* Bit mask for the R field in the encoding */
#define E_BIT 1  /* Bit mask for the E field in the encoding */

/* Encode a label address into a binary format, including its external/internal status */
unsigned short encode_label_address(int address, int is_external) {
    /* Shift the address left by 3 bits to make room for the encoding bits */
    unsigned short binary_value = (address << 3); 

    /* If the label is external, set the E bit */
    if (is_external) {
        binary_value |= E_BIT;
    } else {
        /* If the label is internal, set the R bit */
        binary_value |= R_BIT;
    }

    /* Return the encoded binary value */
    return binary_value;
}

/* Check if a given operand is a label, either in the internal label table or the external label table */
int is_label(const char *operand, label_table *table, label_table *externs) {
    int i;
    
    /* Check if the operand starts with an alphabetic character */
    if (!isalpha(operand[0])) {
        return 0;  /* Not a label if it doesn't start with an alphabetic character */
    }

    /* Check if the operand is a register (e.g., r0 to r7) */
    if (operand[0] == 'r' && operand[1] >= '0' && operand[1] <= '7' && operand[2] == '\0') {
        return 0;  /* Not a label if it is a register */
    }
    
    /* Check if the operand matches any label in the internal label table */
    for (i = 0; i < table->count; i++) {
        if (strcmp(operand, table->labels[i].name) == 0) {
            return 1;  /* Operand is a label found in the internal table */
        }
    }

    /* Check if the operand matches any label in the external label table */
    for (i = 0; i < externs->count; i++) {
        if (strcmp(operand, externs->labels[i].name) == 0) {
            return 1;  /* Operand is a label found in the external table */
        }
    }
    
    return 0;  /* Operand is not a label in either table */
}

/* Check if a given string represents a register */
int is_register(const char *str) {
    /* Check if the input string is NULL or too short to be a register */
    if (str == NULL || strlen(str) < 2) {
        return 0;  /* Not a register if the string is NULL or too short */
    }

    /* Check if the string matches the format for a register (e.g., r0 to r7) */
    if (str[0] == 'r' && isdigit(str[1]) && str[2] == '\0') {
        return 1;  /* The string is a valid register (e.g., r0, r1, ..., r7) */
    } 
    /* Check if the string matches the format for an indirect register (e.g., *r0) */
    else if (str[0] == '*' && str[1] == 'r' && isdigit(str[2]) && str[3] == '\0') {
        return 1;  /* The string is a valid indirect register (e.g., *r0, *r1, ..., *r7) */
    }

    return 0;  /* The string does not match any register format */
}

/* Parse operands for labels, update instruction encoding, and handle external labels */
void parse_operands_for_labels(const char *operands, label_table *table, label_table *extern_entry, code_conv *instructions, int *IC, location *am_file, external_label_table *externals) {
    char *operand_copy;
    char *token;
    label *label_info, *is_extern;
    size_t operands_len;
    int operand_count = 0, register_found = 0;

    /* Get the length of the operands string */
    operands_len = strlen(operands);
    
    /* Allocate memory for a copy of the operands string */
    operand_copy = (char *)malloc(operands_len + 1);
    if (operand_copy == NULL) {
        printf("Memory allocation failed\n");
        return;  /* Exit if memory allocation fails */
    }
    strcpy(operand_copy, operands);

    /* Tokenize the operands string by commas */
    token = strtok(operand_copy, ",");
    while (token != NULL) {
        /* Remove leading and trailing whitespace from the token */
        token = trim_whitespace(token);
        
        /* Check if the token is a label */
        if (is_label(token, table, extern_entry)) {
            int is_external = 0;

            /* Find label information in the internal label table */
            label_info = find_label(table, token);
            /* Find label information in the external label table */
            is_extern = find_label(extern_entry, token);
            
            /* If the label is external, add it to the external label table */
            if (is_extern && is_extern->is_external) {
                is_external = 1;

                /* Resize the external labels table if needed */
                if (externals->count >= externals->capacity) {
                    externals->capacity *= 2;
                    externals->labels = realloc(externals->labels, externals->capacity * sizeof(label));
                    if (externals->labels == NULL) {
                        printf("Memory allocation failed\n");
                        free(operand_copy);
                        return;  /* Exit if memory reallocation fails */
                    }
                }

                /* Add the external label to the table */
                externals->labels[externals->count].address = *IC + 100 + operand_count;
                strcpy(externals->labels[externals->count].name, token);
                externals->labels[externals->count].is_external = 1;
                externals->count++;
            }

            /* Encode the label address in the instructions array */
            if (label_info) {
                instructions[*IC + operand_count].binary_repres = encode_label_address(label_info->address, is_external);
            } else if (is_extern) {
                instructions[*IC + operand_count].binary_repres = encode_label_address(is_extern->address, is_external);
            } else {
                /* Print an error if the label is not found */
                PRINT_ERROR1(am_file->file_name, am_file->line, "Label '%s' not found in the label table.\n", token);
            }
        }
        
        /* Check if the token is a register */
        if (is_register(token)) {
            if (register_found) {
                /* Decrement operand_count if a second register is found */
                operand_count--;
            } else {
                register_found = 1;
            }
        }
        
        /* Increment operand count */
        operand_count++;
        token = strtok(NULL, ",");  /* Get the next token */
    }

    /* Update the instruction counter */
    *IC += operand_count;

    /* Free the allocated memory for operand_copy */
    free(operand_copy);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int references_external_label(const code_conv *instruction, const char *label_name);

char **get_external_labels(label_table *table, int *external_count) {
    int i, count;
    char **external_labels;

    /* Initialize count and external_labels */
    count = 0;
    external_labels = NULL;

    /* First, count the number of external labels */
    for (i = 0; i < table->count; i++) {
        if (table->labels[i].is_external) {
            count++;
        }
    }

    /* Allocate memory for the array of strings */
    external_labels = (char **)malloc(count * sizeof(char *));
    if (!external_labels) {
        printf("Memory allocation failed\n");
        *external_count = 0;
        return NULL;
    }

    /* Copy the names of external labels */
    count = 0;
    for (i = 0; i < table->count; i++) {
        if (table->labels[i].is_external) {
            external_labels[count] = (char *)malloc(strlen(table->labels[i].name) + 1);
            if (!external_labels[count]) {
                int j;
                printf("Memory allocation failed\n");
                /* Free previously allocated memory before returning NULL */
                for (j = 0; j < count; j++) {
                    free(external_labels[j]);
                }
                free(external_labels);
                *external_count = 0;
                return NULL;
            }
            strcpy(external_labels[count], table->labels[i].name);
            count++;
        }
    }

    *external_count = count;
    return external_labels;
}


char *get_line_from_file(int line_number, FILE *file) {
    int current_line = 1;
    char *line = NULL;
    size_t len = 0;
    char buffer[1024];

    if (line_number < 1 || file == NULL) {
        return NULL;
    }

    /* Move to the beginning of the file */
    rewind(file);

    /* Read the file line by line until the desired line is found */
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        if (current_line == line_number) {
            len = strlen(buffer);
            line = (char *)malloc((len + 1) * sizeof(char));
            if (line == NULL) {
                return NULL; /* Memory allocation failed */
            }
            strcpy(line, buffer);
            return line;
        }
        current_line++;
    }

    /* If the function reaches this point, the desired line was not found */
    return NULL;
}

/* Create output files for object code, entry labels, and external labels */
void create_output_files(const char *filename_with_ext, code_conv *instructions, int IC, code_conv *data, int DC, label_table *labels, external_label_table *externals, FILE *am_file, label_table *extern_entry) {
    char base_filename[FILENAME_MAX];
    char obj_filename[FILENAME_MAX];
    char ent_filename[FILENAME_MAX];
    char ext_filename[FILENAME_MAX];
    FILE *obj_file, *ent_file, *ext_file;
    int has_entries = 0, has_externals = 0, external_count;
    int i;
    label *lbl, *lbl_copy;
    size_t len;
    char **external_label_names;
    
    /* Get the list of external label names */
    external_label_names = get_external_labels(extern_entry, &external_count);
    
    /* Remove the .am extension to get the base filename */
    strncpy(base_filename, filename_with_ext, FILENAME_MAX - 1);
    len = strlen(base_filename);
    if (len > 3 && strcmp(base_filename + len - 3, ".am") == 0) {
        base_filename[len - 3] = '\0';  /* Remove the last 3 characters (.am) */
    }

    /* Create output filenames by appending appropriate extensions */
    if (strlen(base_filename) + 3 < FILENAME_MAX) {
        sprintf(obj_filename, "%s.ob", base_filename); /* Object file */
        sprintf(ent_filename, "%s.ent", base_filename); /* Entries file */
        sprintf(ext_filename, "%s.ext", base_filename); /* Externals file */
    } else {
        fprintf(stderr, "Filename too long\n");
        return;  /* Exit if the filename is too long */
    }

    /* Create and open the object file for writing */
    obj_file = fopen(obj_filename, "w");
    if (!obj_file) {
        perror("Error creating object file");
        return;  /* Exit if the object file cannot be created */
    }

    /* Write the header to the object file, including instruction and data counts */
    fprintf(obj_file, "%d %d\n", IC, DC);

    /* Write the instructions to the object file */
    for (i = 0; i < IC; i++) {
        fprintf(obj_file, "%04d %05o\n", 100 + i, instructions[i].binary_repres);
    }

    /* Write the data segment to the object file */
    for (i = 0; i < DC; i++) {
        fprintf(obj_file, "%04d %05o\n", 100 + IC + i, data[i].binary_repres);
    }
    fclose(obj_file);

    /* Create and open the entries file if there are entries */
    for (i = 0; i < extern_entry->count; i++) {
        lbl = &extern_entry->labels[i];
        if (lbl->is_entry) {
            if (!has_entries) {
                ent_file = fopen(ent_filename, "w");
                if (!ent_file) {
                    perror("Error creating entries file");
                    return;  /* Exit if the entries file cannot be created */
                }
                has_entries = 1;
            }
            /* Find the label information and write it to the entries file */
            lbl_copy = find_label(labels, lbl->name);
            fprintf(ent_file, "%s %d\n", lbl->name, lbl_copy->address);
        }
    }
	if (externals->count > 0) {
    	/* Create and open the externals file */
    	ext_file = fopen(ext_filename, "w");
    	if (!ext_file) {
        	perror("Error creating externals file");
        	return;  /* Exit if the externals file cannot be created */
    	}
		has_externals = 1;
		
		/* Write the external label references and their addresses to the externals file */
		for (i = 0; i < externals->count; i++) {
		    label *lbl = &externals->labels[i];
		    fprintf(ext_file, "%s %d\n", lbl->name, lbl->address);
		}
	}

    /* Free memory allocated for external label names */
    for (i = 0; i < external_count; i++) {
        free(external_label_names[i]);  /* Free each label name */
    }
    
    /* Close the entries and externals files if they were created */
    if (has_entries) {
        fclose(ent_file);
    }
    if (has_externals) {
        fclose(ext_file);
    }
}
