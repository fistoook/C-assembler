#ifndef SECOND_PASS_H
#define SECOND_PASS_H

void execute_second_pass(FILE *source_file, code_conv *instructions, code_conv *data, label_table labels, location *am_file, int *cc_capacity, int DC, label_table extern_entry);

#endif
