#include "intialize_data_struct.h"

void initialize_label_table(label_table *table) {
    table -> labels = (label *) malloc(sizeof(label) * INTIAL_AMOUNT_OF_LABELS);
    if (!table -> labels) {
        printf("MEMORY ALLOCATION FAILED");
        exit(1);
    }
    table -> count = 0;
    table -> capacity = INTIAL_AMOUNT_OF_LABELS;
}

void free_label_table(label_table *table) {
    free(table -> labels);
    table -> labels = NULL;
    table -> count = 0;
    table -> capacity = 0;
}

void initialize_location(location **am_file, char *am_file_name) {
    *am_file = (location *)malloc(sizeof(location));
    if (*am_file == NULL) {
         printf("MEMORY ALLOCATION FAILED");
         exit(1);
    }
    
    (*am_file)->file_name = (char *)malloc(strlen(am_file_name) + 1);
    if ((*am_file)->file_name == NULL) {
        printf("MEMORY ALLOCATION FAILED");
        exit(1);
    }
    strcpy((*am_file)->file_name, am_file_name);
    
    (*am_file)->line = 0;
}
void initialize_code_conv(code_conv **cc) {
    int i, j;
    *cc = (code_conv *)malloc(INITIAL_CC_CAPACITY * sizeof(code_conv));
    if (!(*cc)) {  
        printf("MEMORY ALLOCATION FAILED");
        exit(1);
    }
  
    for (i = 0; i < INITIAL_CC_CAPACITY; i++) {
        (*cc)[i].binary_repres = 0;
        for (j = 0; j < INTIAL_AMOUNT_OF_LABELS; j++) {
            (*cc)[i].label[j] = NULL;
        }
        (*cc)[i].assembly_line = 0;
    }
}
/* Function to free the allocated memory for a code_conv pointer */
void free_code_conv(code_conv *data) {
    if (data) {
        if (data -> label) {
            free(data -> label);
        }
        free(data);
    }
}

void initialize_instruction(instruction **instr) {
	int i;
     *instr = (instruction *)malloc(sizeof(instruction));
     if (*instr == NULL) {
          printf("Memory allocation failed!\n");
          exit(1);
    }
    

    (*instr)->opcode = 0;
    (*instr)->operand_count = 0;
    (*instr)->length = 0;
    (*instr)->binary_repres = 0;

    for (i = 0; i < MAX_OPERANDS; i++) {
        (*instr)->operands[i].type = 0;
        (*instr)->operands[i].value = 0;
        (*instr)->operands[i].is_label = 0;
        (*instr)->operands[i].is_register = 0;
        (*instr)->operands[i].register_index = -1;
    }
}

void initialize_external_label_table(external_label_table *externals) {
    externals->labels = malloc(10 * sizeof(label)); 
    if (externals->labels == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    externals->count = 0;
    externals->capacity = 10;
}
