#include "assembler.h"
#include "intialize_data_struct.h"

int main(int argc, char *argv[]) {
    char *input_file_name, *as_file_name, *am_file_name;
    int first_pass_success;
    int i;

    /* Step 1: Check command-line arguments */
    if (argc < 2) {
        printf("Usage: %s <input_file_name(s)>\n", argv[0]);
        return 1;
    }

    /* Step 2: Loop over all input files provided as arguments */
    for (i = 1; i < argc; i++) {
        /* Get the input file name without an extension */
        input_file_name = argv[i];

        /* Prepare the name for the .as file (for macro extension) */
        as_file_name = malloc(strlen(input_file_name) + 4); /* Allocate space for ".as" extension */
        if (!as_file_name) {
            printf("Memory allocation failed\n");
            return 1;
        }

        strcpy(as_file_name, input_file_name);
        strcat(as_file_name, ".as");

        /* Print starting macro extension */
        printf("Starting macro extension for file: %s\n", as_file_name);

        /* Call macro_extender (assumed to be defined elsewhere) */
        macro_extender(as_file_name);
        /* Print success of macro extension */
        printf("Macro extension succeeded for file: %s\n", as_file_name);

        /* Prepare the name for the .am file (output from macro_extender) */
        am_file_name = malloc(strlen(input_file_name) + 4); /* Allocate space for ".am" extension */
        if (!am_file_name) {
            printf("Memory allocation failed\n");
            free(as_file_name);
            return 1;
        }

        strcpy(am_file_name, input_file_name);
        strcat(am_file_name, ".am");

        /* Print starting first pass */
        printf("Starting first pass for file: %s\n", am_file_name);

        /* Execute the first pass on the .am file */
        first_pass_success = execute_first_pass(am_file_name);

        /* Print the result of the first pass */
        if (first_pass_success) {
            printf("First pass completed successfully for file: %s\n", am_file_name);
        } else {
            printf("First pass encountered errors for file: %s\n", am_file_name);
        }

        /* Free allocated memory */
        free(as_file_name);
        free(am_file_name);
    }

    /* Return 0 if all files succeeded, 1 if any file failed */
    return first_pass_success ? 0 : 1;
}
