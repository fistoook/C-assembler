#include "pre_assembler.h"
#include "util.h"

/* this function will extend the macros in the assembly code
   by iterating through the source file after opening it.
   after creating the output file, with each iteration
   the function identifies one of this case:
   1) new macro declaration. in this case the function checks
      if the name is legal and if there is no text after the macro
      definition. if so, the function adds the macro to the
      macro table and updates the macros name and allocates
      memory for the macro lines(which will be set in the next
      iterations). the declaration and the line of the macro are
      not copied to the output file .am.
   2) currently inside of a macro. after a macro definition was
      encounetrd, the lines of the macro need to be copied into
      the macro table one by one until the end of macro. it is
      exactly what the function does. checks for end of macro,
      copies the line and then continues.
   3) a macro call. the function checks if a macro call
      was encounterd by comparing the first word in the senctence
      to all of the macros in the macro table. if a call is indeed
      found, the call is replaced with the macro lines.
   4) non of the mentiond above. if a simple line encounterd
      (a line that has nothing to do with macros) it is just copied
      to the output file as is.
   in the end we get the output file with the macros extended.
   comments and empty lines are ignored by the compile,
   so in the .am file they will not be present.
*/

FILE *macro_extender(const char *source_file_name) {
    FILE *source_file = fopen(source_file_name, "r"); /* open source file for reading */
    FILE *output_file; /* the output file which wilol store the end result */
    char next_line[MAX_LINE_LENGTH + 2]; /* the line that we will read from the source file */
    MacroTable macro_table = {NULL}; /* the macro_table */
    Macro *current_macro = NULL;
    int inside_macro = 0; /* a flag for when inside a macro */
    int line_number = 0; /* line counter */

    if (!source_file) {
        printf("Can not access source file. Stop!\n");
        return NULL;
    }

	macro_table.head = malloc(sizeof(Macro));
    if (!macro_table.head) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    macro_table.head -> next = NULL;
    macro_table.head -> name = NULL;
    macro_table.head -> lines = NULL;
    macro_table.head -> line_count = 0;
    macro_table.head -> capacity = 0;

    output_file = create_output_file(source_file_name);
    if (!output_file) {
    	fclose(source_file);
        return NULL;
    }
    if (check_line_lengths(source_file_name)) {
        fclose(source_file);
        fclose(output_file);
        return NULL;
    }

    while (fgets(next_line, sizeof(next_line), source_file)) { /* get line from file till EOF reached */
    	char *first_word;
    	int len, j, macro_flag = 0;
        line_number++;
    	if (is_empty_line(next_line)) /* check if line empty, ignore it */
    		continue;

    	remove_leading_whitespace(next_line);
    	if (next_line[0] == ';') /* check if line is a comment, ignore it */
    		continue;

    	first_word = find_word(next_line, 0); /* identify the first word */

        if (inside_macro) { /* if we are reading macro line after def */
            if (first_word && strcmp(first_word, "endmacr") == 0) {
                /* if found ending of macro */
                char *pos = strstr(next_line, first_word) + strlen(first_word); /* find first position after "endmacr" */
                if (only_space_remain(pos)) { /* check if there is no text after the "endmacr" */
                    inside_macro = 0; /* if there is not, continue to next line */
                    current_macro = NULL;
                    free(first_word); /* free memory allocated */
                    continue;
                }
                /* extraneous text */
                printf("error: on line %d extraneous text after macro def. the program will stop now!\n", line_number);
                free(first_word); /* free memory allocated */
                fclose(source_file); /* close files */
                fclose(output_file);
                free_macro_table(&macro_table); /* free the macro table list */
                return NULL; /* found error, now point in continuing. indicate main to go to next file */
            }

            /* Inside a macro definition, add the line to the current macro */
            if (current_macro -> line_count == current_macro -> capacity) {
                /* Reallocate memory for macro lines if needed */
                current_macro -> capacity *= 2;
                current_macro -> lines = realloc(current_macro -> lines, current_macro -> capacity * sizeof(char *));
                if (!current_macro -> lines) {
                    printf("Memory allocation failed\n");
                    exit(1);
                }
            }

			remove_leading_whitespace(next_line);
            len = strlen(next_line) + 1; /* +1 for the null terminator */
            current_macro -> lines[current_macro ->line_count] = malloc(len); /* allocate memory for current line */
            if (!current_macro -> lines[current_macro -> line_count]) {
                printf("Memory allocation failed\n");
                exit(1);
            }
            memcpy(current_macro -> lines[current_macro -> line_count++], next_line, len); /* copy the line */
            free(first_word);
            continue;
        }

        if (first_word && strcmp(first_word, "macr") == 0) { /* found new macro definition */
            char *macro_name, *pos;
            int name_len;
            remove_leading_whitespace(next_line);
            if (!only_space_remain(next_line + 4))
                macro_name = find_word(next_line, 4);
            else {
                /* if the macro doesnt have a name, it is useless. so we just continue iterating */
                printf("warning: on line %d macro defintion has no effect. no name was provided.", line_number);
                free(first_word);
                continue;
            }

            if (!is_legal_macro(macro_name)) { /* cheak if the macro is not named after a directive or an instruction */
                printf("error: on line %d Illegal macro name %s! The program will stop now.\n", line_number, macro_name);
                free(first_word); /* free all the memory allocated */
                free(macro_name);
                fclose(source_file); /* close the files */
                fclose(output_file);
                free_macro_table(&macro_table); /* free the macro table list */
                return NULL;  /* indicate main that macro extension failed, go on to next file */
            }

			pos = strstr(next_line, macro_name); /* find the position where the macro name starts */
            pos += strlen(macro_name); /* find the first char after the macro name */

            if (!only_space_remain(pos)) { /* text after definetion */
                printf("error: on line %d Extraneous text after macro def. The program will stop now!\n", line_number);
                free(first_word); /* free all the memory allocated */
                free(macro_name);
                fclose(source_file); /* close the files */
                fclose(output_file);
                free_macro_table(&macro_table); /* free the macro table list */
                return NULL; /* indicate main that macro extension failed, go on to next file */
            }

			if (!macro_table.head) {
                macro_table.head = malloc(sizeof(Macro));
                if (!macro_table.head) {
                    printf("Memory allocation failed\n");
                    exit(1);
                }
                macro_table.head -> next = NULL;
                macro_table.head -> name = NULL;
                macro_table.head -> lines = NULL;
                macro_table.head -> line_count = 0;
                macro_table.head -> capacity = 0;
            }

            inside_macro = 1; /* turn on inside_macro flag */
            current_macro = macro_table.head; /* add macro to table */
            while (current_macro -> next)
                current_macro = current_macro -> next;

            if (current_macro -> name) {
                current_macro -> next = malloc(sizeof(Macro));
                if (!current_macro -> next) {
                    printf("Memory allocation failed\n");
                    exit(1);
                }
                current_macro = current_macro -> next;
                current_macro -> next = NULL;
            }

            name_len = strlen(macro_name) + 1;
            current_macro -> name = (char *) malloc(name_len);
            if (!current_macro -> name) {
                printf("Memory allocation failed\n");
                exit(1);
            }
            memcpy(current_macro -> name, macro_name, name_len);

            current_macro -> lines = malloc(sizeof(char *) * 100);
            if (!current_macro -> lines) {
                printf("Memory allocation failed\n");
                exit(1);
            }
            current_macro -> line_count = 0;
            current_macro -> capacity = 100;
            free(macro_name);
            free(first_word);
            continue;
        }

        /* check if the first word is a call to a macro previously defined */
        current_macro = macro_table.head;
        while (current_macro != NULL) {
            if (current_macro -> name && strcmp(first_word, current_macro -> name) == 0) {  /* compare the word to each macro name */
                for (j = 0; j < current_macro -> line_count; j++) /* found call for macro, replace with macro lines */
                    fprintf(output_file, "%s", current_macro -> lines[j]);
                macro_flag = 1;
                break;
            }
            current_macro = current_macro -> next;
        }
        if (macro_flag) {
        	free(first_word);
            continue;
        }

        fprintf(output_file, "%s", next_line); /* copy non-macro lines as is */
        free(first_word); /* free allocated memory before next iteration */
    }
    fclose(source_file); /* close both files */
    fclose(output_file);
    free_macro_table(&macro_table);
    return output_file; /* return the output file (a pointer to it) */
}

FILE *create_output_file(const char *source_file_name) {
    FILE *output_file; /* the output file */
    char *output_file_name; /* name of the output file */
    int len = strlen(source_file_name) + 1; /* +1 for the null terminator */
    output_file_name = (char *)malloc(len); /* allocate memory */
    if (!output_file_name) {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    strcpy(output_file_name, source_file_name); /* Copy the source file name to the new memory */
    strcpy(output_file_name + len - 4, ".am"); /* add ".am" instead of .as */

    output_file = fopen(output_file_name, "w"); /* create the file */
    if (!output_file) {
        printf("Can not create output file\n");
        free(output_file_name);
        return NULL;
    }

    free(output_file_name); /* Free allocated memory */
    return output_file; /* Return the output file (a pointer to it to) */
}

/* this function is to check that the macro name is legal
 * by iterating thourgh a matrix of illegal macro names
 * that is defined in the header and comparing each illegal
 * name to the one in the code it deicdes if the name
 * is indeed legal
*/
int is_legal_macro(const char *macro) {
    int num_illegal_macros = sizeof(illegal_macro_names) / sizeof(illegal_macro_names[0]), i; /* calculate the number of memebers in matrix */
    for (i = 0; i < num_illegal_macros; i++)  /* iterate through the matrix */
        if (strcmp(macro, illegal_macro_names[i]) == 0)  /* compare ilegaames) l name with the macro name */
            return 0; /* Macro name is illegal */
    return 1; /* Macro name is legal */
}


/* this function will free all the memory
 * allocated for the macro table in macro
 * extender function
*/
void free_macro_table(MacroTable *macro_table) {
    Macro *current = macro_table -> head; /* to iterate the list take the head */
    int i;
    while (current) { /* while not at the end */
        Macro *next = current -> next; /* take the next node and keep it */
        free(current -> name); /* free the macro name */
        for (i = 0; i < current -> line_count; i++) { /* free each macro line */
            free(current -> lines[i]);
        }
        free(current -> lines); /* free the array of lines */
        free(current); /* free the node */
        current = next; /* move to next */
    }
    macro_table -> head = NULL; /* the list is emptyt now */
}
