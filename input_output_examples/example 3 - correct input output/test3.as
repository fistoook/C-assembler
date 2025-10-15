MAIN: add r3, LIST
LOOP: prn #48
macr m_macr
cmp r3, #-6
bne END
endmacr
lea STR, r6
inc r6
mov *r6,K

;comment

sub r1, r4
m_macr

dec K
jmp LOOP
END: stop
STR: .string "abcd"
LIST: .data 6, -9
.data -100
K: .data 31

macr M2
mov reg1, val
add reg2, reg1
    endmacr

M2

ABC: mov r2, #1000
