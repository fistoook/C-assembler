#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define MAX_LINE_LENGTH 80

/* this function finds the next word in a line from a given index */
char *find_word(char *line, int start) {
    int end;
    char *word;
    int word_length;

    /* Check if the input line is NULL */
    if (line == NULL) 
        return NULL;

    /* Skip leading whitespace from the start index */
    while (line[start] != '\0' && isspace((unsigned char)line[start])) 
        start++;
    
    /* If the end of the line is reached, no word found */
    if (line[start] == '\0') 
        return NULL;

    /* Find the end of the word by scanning until a whitespace or end of line is encountered */
    end = start;
    while (line[end] != '\0' && !isspace((unsigned char)line[end])) 
        end++;
    
    /* Calculate the length of the word */
    word_length = end - start;

    /* Allocate memory for the word, including space for the null terminator */
    word = (char *) malloc(word_length + 1);
    if (!word) {
        printf("MEMORY_ALLOCATION_FAILED");
        return NULL;
    }
    
    /* Copy the word from the line to the allocated memory */
    memcpy(word, line + start, word_length);
    word[word_length] = '\0'; /* Null-terminate the copied word */

    return word;
}

/* this function checks if from a point in a line there is no other chars */
int only_space_remain(const char *pos) {
    while (*pos) { /* while have not reached end of string */
        if (!isspace(*pos)) return 0; /* found char */
        pos++; /* continue to next one */
    }
    return 1; /* no other char was found, return true */
}

/* this function decides whether on not a line is empty */
int is_empty_line(const char *line) {
    while (*line) { /* while have not reached end of string */
        if (!isspace(*line)) /* if not empty return false */
            return 0;
        line++; /* continue to next one */
    }
    return 1; /* we are here, so the line is empty. return true */
}

/* This function will remove whitespaces at the start of a string */
void remove_leading_whitespace(char *str) {
    char *start = str, *dest;
    
    /* Skip over leading spaces */
    while (isspace((unsigned char)*start)) 
        start++;

    /* If there are leading spaces, move the remaining characters to the start of the string */
    if (start != str) {
        dest = str;
        while (*start)
            *dest++ = *start++;
        *dest = '\0'; /* Null-terminate the modified string */
    }
}

/* this function will check if there is a line that is too long in the file */
int check_line_lengths(const char *file_name) {
	char line[MAX_LINE_LENGTH + 2]; /* +2 for null and \n */
    int line_number = 0; /* to count lines */
    
    FILE *file = fopen(file_name, "r"); /* open to read */
    if (!file) {
        printf("Cannot open file %s\n", file_name);
        return -1;
    }
   
    /* get lines */
    while (fgets(line, sizeof(line), file)) {
        line_number++; /* count for error printing */
        if (strchr(line, '\n') == NULL && !feof(file)) { /* a line which is too long detcted */
            printf("Error on line %d: to much charchters. the maximum line length is 80!\n", line_number); /* print error */
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

char *trim_whitespace(char *str) {
    char *end;

    /* Trim leading space */
    while (isspace(*str)) str++;

    if (*str == 0)  /* All spaces? */
        return str;

    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    /* Write new null terminator */
    *(end + 1) = 0;

    return str;
}


/* This function prints an error message to the standard output with the file name and line number where the error occurred.
   The error message is formatted using the provided format string and additional arguments. */
void print_error(const char *file, int line, const char *format, ...) {
    va_list args;  /* Variable to hold the list of arguments */
    
    va_start(args, format);  /* Initialize the argument list */
    
    /* Print the error message header in red text */
    printf("\033[1;31m~~ERROR: File: %s, Line: %d, ", file, line);
    
    /* Print the formatted error message */
    vprintf(format, args);
    
    /* End the error message with a footer in red text */
    printf("~~\033[0m\n");
    
    va_end(args);  /* Clean up the argument list */
}

/* This function prints a warning message to the standard output with the file name and line number where the warning occurred.
   The warning message is formatted using the provided format string and additional arguments. */
void print_warning(const char *file, int line, const char *format, ...) {
    va_list args;  /* Variable to hold the list of arguments */
    
    va_start(args, format);  /* Initialize the argument list */
    
    /* Print the warning message header in blue text */
    printf("\033[1;34m~~WARNING: File: %s, Line: %d, ", file, line);
    
    /* Print the formatted warning message */
    vprintf(format, args);
    
    /* End the warning message with a footer in blue text */
    printf("~~\033[0m\n");
    
    va_end(args);  /* Clean up the argument list */
}

char* my_strdup(const char* s) {
    char* d = malloc(strlen(s) + 1);  /* Space for length plus null */
    if (d == NULL) return NULL;       /* No memory */
    strcpy(d, s);                     /* Copy the characters */
    return d;                         /* Return the new string */
}
