# Build the final executable
assembler: assembler.o first_pass.o code_conversion.o parser.o intialize_data_struct.o util.o pre_assembler.o second_pass.o 
	gcc -g -Wall -ansi -pedantic -o assembler assembler.o first_pass.o code_conversion.o parser.o intialize_data_struct.o util.o pre_assembler.o second_pass.o 

# Compile assembler.c to assembler.o
assembler.o: assembler.c assembler.h globals.h
	gcc -g -Wall -ansi -pedantic -c assembler.c

# Compile first_pass.c to first_pass.o
first_pass.o: first_pass.c first_pass.h globals.h second_pass.h
	gcc -g -Wall -ansi -pedantic -c first_pass.c

# Compile code_conversion.c to code_conversion.o
code_conversion.o: code_conversion.c code_conversion.h globals.h
	gcc -g -Wall -ansi -pedantic -c code_conversion.c

# Compile parser.c to parser.o
parser.o: parser.c parser.h globals.h
	gcc -g -Wall -ansi -pedantic -c parser.c

# Compile intialize_data_struct.c to intialize_data_struct.o
intialize_data_struct.o: intialize_data_struct.c intialize_data_struct.h globals.h
	gcc -g -Wall -ansi -pedantic -c intialize_data_struct.c

# Compile util.c to util.o
util.o: util.c util.h globals.h
	gcc -g -Wall -ansi -pedantic -c util.c

# Compile pre_assembler.c to pre_assembler.o
pre_assembler.o: pre_assembler.c pre_assembler.h globals.h
	gcc -g -Wall -ansi -pedantic -c pre_assembler.c

# Compile second_pass.c to second_pass.o
second_pass.o: second_pass.c first_pass.h intialize_data_struct.h parser.h util.h globals.h
	gcc -g -Wall -ansi -pedantic -c second_pass.c
	
# Clean up build files
clean:
	rm -f assembler assembler.o first_pass.o code_conversion.o parser.o intialize_data_struct.o util.o pre_assembler.o

