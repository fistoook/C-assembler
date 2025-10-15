#ifndef UTIL_H
#define UTIL_H

/* 
 * Finds the next word in a string starting from a given index.
 * The word is defined as a sequence of non-whitespace characters.
 * 
 * Parameters:
 *   line - The string to search within.
 *   start - The index to start searching from.
 * 
 * Returns:
 *   A dynamically allocated string containing the next word, or NULL if no word is found.
 */
char *find_word(const char *line, int start);

/* 
 * Checks if a string contains only whitespace characters.
 * 
 * Parameters:
 *   str - The string to check.
 * 
 * Returns:
 *   1 if the string contains only whitespace characters, 0 otherwise.
 */
int only_space_remain(const char *str);

/* 
 * Checks if a line is empty, meaning it contains no characters except for whitespace.
 * 
 * Parameters:
 *   line - The line to check.
 * 
 * Returns:
 *   1 if the line is empty, 0 otherwise.
 */
int is_empty_line(const char *line);

/* 
 * Removes leading whitespace characters from a string.
 * 
 * Parameters:
 *   str - The string from which leading whitespace will be removed.
 */
void remove_leading_whitespace(char *str);

/* 
 * Checks if a line exceeds a certain length.
 * 
 * Parameters:
 *   line - The line to check.
 * 
 * Returns:
 *   1 if the line length exceeds the limit, 0 otherwise.
 */
int check_line_lengths(const char *line);

/* 
 * Trims leading and trailing whitespace characters from a string.
 * 
 * Parameters:
 *   str - The string to trim.
 * 
 * Returns:
 *   A dynamically allocated string with leading and trailing whitespace removed.
 */
char *trim_whitespace(char *str);

/* 
 * Duplicates a string by allocating memory for a new string and copying the content of the given string.
 * 
 * Parameters:
 *   s - The string to duplicate.
 * 
 * Returns:
 *   A dynamically allocated copy of the input string.
 */
char *my_strdup(const char *s);

#endif
