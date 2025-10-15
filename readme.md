# 🛠️ C Assembler

A **two-pass assembler** implemented in **C** for a custom assembly language. 
This assembler converts human-readable assembly code into binary machine code, 
handling instructions, operands, labels, and data with proper encoding.

---

## 🚀 Features

- **Two-pass assembly process**  
  Ensures labels and addresses are correctly resolved across multiple passes.
  
- **Supports multiple operand types**  
  - Immediate values (e.g., `#5`)  
  - Direct addressing with labels  
  - Direct registers (`r0`–`r7`)  
  - Indirect register pointers (`*r0`–`*r7`)

- **Advanced error handling**  
  Detects and reports:
  - Syntax errors  
  - Illegal operands or instructions  
  - Invalid comma usage and other formatting mistakes
  - and more, including warnings...

- **Binary encoding scheme**  
  - 15-bit word encoding for instructions and data  
  - Opcode, addressing methods, and A/R/E bits properly set  
  - Efficient handling for register-to-register operations

- **Extensible and modular**  
  Easily add new opcodes, addressing modes, or instruction types.
  
## 📝 Usage

I provided a complete **MAKEFILE** to help you compile the assembler.
Just go ahead and run the program, you will be fully guided and instucted and the proper usage!

## 🌟 Acknowledgements

This is the final project (Maman 14) for the Systems Programming Laboratory course at the Open University of Israel

