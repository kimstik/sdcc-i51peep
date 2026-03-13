// tests.c - Consolidated peephole notUsed testbench for MCS51
//
// 11 positive (SHOULD FIRE) + 16 negative (MUST NOT FIRE) = 27 tests.
// Each function is __naked with inline asm for exact instruction control.
// Label barriers prevent standard peephole rules from interfering.
// SFR writes (mov 0x80/0x81,rX) prevent dead code elimination.
//
// Compile: sdcc -mmcs51 --peep-asm --peep-file tests.def -S -o tmp/tests.asm tests.c
// Verify:  bash check.sh tmp/tests.asm

// lcall target for test_carry_dead_at_lcall
void dummy(void) __naked { __asm ret __endasm; }

// =====================================================================
// SHOULD FIRE - notUsed() returns TRUE, rule fires
// =====================================================================

// 01: notUsed('a')=TRUE - acc dead after mov r0,a (nothing reads A)
void test_acc_basic(void) __naked {
    __asm
    mov  a,#0x42
    mov  r0,a
00011$:
    mov  0x80,r0
    ret
    __endasm;
}

// 02: notUsed('c')=TRUE - add writes carry, nothing reads it
void test_carry_dead_after_add(void) __naked {
    __asm
    add  a,#0x10
    mov  r0,a
00021$:
    mov  0x80,r0
    ret
    __endasm;
}

// 03: notUsed('c')=TRUE - jb ov reads OV, not CY (PSW_IDX vs CND_IDX split)
void test_carry_not_blocked_by_ov(void) __naked {
    __asm
    add  a,r0
    mov  r1,a
00031$:
    jb   ov,00032$
    mov  0x80,r1
    ret
00032$:
    mov  0x81,r1
    ret
    __endasm;
}

// 04: notUsed('c')=TRUE - carry dead at function call boundary
void test_carry_dead_at_lcall(void) __naked {
    __asm
    add  a,#0x20
    mov  r0,a
00041$:
    lcall _dummy
    mov  0x80,r0
    ret
    __endasm;
}

// 05: notUsed('c')=TRUE - cjne writes carry, nothing reads it
void test_carry_dead_after_cjne(void) __naked {
    __asm
    cjne a,#0x10,00052$
00051$:
    mov  0x80,a
    ret
00052$:
    mov  0x80,a
    ret
    __endasm;
}

// 06: notUsed('c')=TRUE - jb p reads parity, not CY (PSW.0 = bit 0xD0)
void test_carry_not_blocked_by_p(void) __naked {
    __asm
    add  a,r0
    mov  r1,a
00061$:
    jb   p,00062$
    mov  0x80,r1
    ret
00062$:
    mov  0x81,r1
    ret
    __endasm;
}

// 07: notUsed('c')=TRUE - second add overwrites carry without reading it
void test_carry_killed_by_second_writer(void) __naked {
    __asm
    add  a,r0
    mov  r1,a
00071$:
    add  a,r1
    mov  0x80,a
    ret
    __endasm;
}

// 08: notUsed('c')=TRUE - jb f0 reads user flag F0, not CY (PSW.5 = bit 0xD5)
void test_carry_not_blocked_by_f0(void) __naked {
    __asm
    add  a,r0
    mov  r1,a
00081$:
    jb   f0,00082$
    mov  0x80,r1
    ret
00082$:
    mov  0x81,r1
    ret
    __endasm;
}

// 09: notUsed('c')=TRUE - mov c,bit writes carry, nothing reads it
void test_carry_write_by_mov_c_bit(void) __naked {
    __asm
    mov  c,0x00
00091$:
    mov  0x80,a
    ret
    __endasm;
}

// 10: notUsed('c')=TRUE - da a writes carry, nothing reads it
void test_carry_dead_after_da(void) __naked {
    __asm
    da   a
00101$:
    mov  0x80,a
    ret
    __endasm;
}

// 11: notUsed('a')=TRUE - movx a,@dptr overwrites A without reading it
void test_acc_dead_after_movx_read(void) __naked {
    __asm
    mov  a,#0x42
    mov  r0,a
00111$:
    movx a,@dptr
    mov  0x80,r0
    ret
    __endasm;
}

// =====================================================================
// MUST NOT FIRE - notUsed() returns FALSE, rule must not fire
// =====================================================================

// 12: notUsed('c')=FALSE - addc reads carry (strstr bug regression)
void test_carry_strstr(void) __naked {
    __asm
    clr  c
00122$:
    addc a,r0
00121$:
    mov  0x80,a
    ret
    __endasm;
}

// 13: notUsed('c')=FALSE - subb reads carry
void test_carry_read_by_subb(void) __naked {
    __asm
    clr  c
00132$:
    subb a,r1
00131$:
    mov  0x80,a
    ret
    __endasm;
}

// 14: notUsed('c')=FALSE - rlc reads+writes carry
void test_carry_read_by_rlc(void) __naked {
    __asm
    setb c
00142$:
    rlc  a
00141$:
    mov  0x80,a
    ret
    __endasm;
}

// 15: notUsed('c')=FALSE - jc reads carry
void test_carry_read_by_jc(void) __naked {
    __asm
    clr  c
00152$:
    jc   00153$
00151$:
    mov  0x80,a
    ret
00153$:
    mov  0x81,a
    ret
    __endasm;
}

// 16: notUsed('c')=FALSE - jnc reads carry
void test_carry_read_by_jnc(void) __naked {
    __asm
    clr  c
00162$:
    jnc  00163$
00161$:
    mov  0x80,a
    ret
00163$:
    mov  0x81,a
    ret
    __endasm;
}

// 17: notUsed('c')=FALSE - rrc reads carry (rotates through CY)
void test_carry_read_by_rrc(void) __naked {
    __asm
    setb c
00172$:
    rrc  a
00171$:
    mov  0x80,a
    ret
    __endasm;
}

// 18: notUsed('c')=FALSE - orl c,bit reads carry (CY = CY | bit)
void test_carry_read_by_orl_c_bit(void) __naked {
    __asm
    clr  c
00182$:
    orl  c,0x00
00181$:
    mov  0x80,a
    ret
    __endasm;
}

// 19: notUsed('c')=FALSE - anl c,bit reads carry (CY = CY & bit)
void test_carry_read_by_anl_c_bit(void) __naked {
    __asm
    clr  c
00192$:
    anl  c,0x00
00191$:
    mov  0x80,a
    ret
    __endasm;
}

// 20: notUsed('c')=FALSE - cpl c reads carry to complement it
void test_carry_read_by_cpl_c(void) __naked {
    __asm
    clr  c
00202$:
    cpl  c
00201$:
    mov  0x80,a
    ret
    __endasm;
}

// 21: notUsed('c')=FALSE - push psw reads entire PSW including carry
void test_carry_read_by_push_psw(void) __naked {
    __asm
    clr  c
00212$:
    push psw
00211$:
    mov  0x80,a
    ret
    __endasm;
}

// 22: notUsed('a')=FALSE - movx @dptr,a reads A
void test_acc_blocked_by_movx_write(void) __naked {
    __asm
    mov  a,#0x42
    mov  r0,a
00221$:
    movx @dptr,a
    ret
    __endasm;
}

// 23: notUsed('a')=FALSE - mul ab reads A
void test_acc_blocked_by_mul(void) __naked {
    __asm
    mov  a,#0x42
    mov  r0,a
00231$:
    mul  ab
    mov  0x80,a
    ret
    __endasm;
}

// 24: notUsed('a')=FALSE - movc a,@a+dptr reads A as index
void test_acc_blocked_by_movc(void) __naked {
    __asm
    mov  a,#0x42
    mov  r0,a
00241$:
    movc a,@a+dptr
    mov  0x80,a
    ret
    __endasm;
}

// 25: notUsed('a')=FALSE - xch a,r1 reads A
void test_acc_blocked_by_xch(void) __naked {
    __asm
    mov  a,#0x42
    mov  r0,a
00251$:
    xch  a,r1
    mov  0x80,a
    ret
    __endasm;
}

// 26: notUsed('a')=FALSE - push acc reads A (direct addr 0xE0)
void test_acc_blocked_by_push(void) __naked {
    __asm
    mov  a,#0x42
    mov  r0,a
00261$:
    push acc
    mov  0x80,r0
    ret
    __endasm;
}

// 27: notUsed('c')=FALSE - carry live across sjmp to addc
void test_carry_live_across_sjmp(void) __naked {
    __asm
    clr  c
00272$:
    sjmp 00274$
00273$:
    ret
00274$:
    addc a,r0
00271$:
    mov  0x80,a
    ret
    __endasm;
}

// sentinel - terminates last function's asm section for check.sh
void test_end(void) __naked { __asm ret __endasm; }
