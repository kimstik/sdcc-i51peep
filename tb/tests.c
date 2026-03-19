// tests.c - Peephole rule testbench for MCS51
//
// Each function contains a pattern that should or should not be optimized
// by peep_mcs51.def rules. Compile with --peep-asm so inline asm is visible.
//
// Compile: sdcc -mmcs51 --peep-asm --peep-file peep_mcs51.def -S -o tb/tmp/tests.asm tb/tests.c
// Verify:  bash tb/check.sh tb/tmp/tests.asm

// helpers
_Noreturn void boot_vec(void) __naked { __asm ljmp 0x3800 __endasm; }

// =====================================================================
// SHOULD FIRE - rule must optimize
// =====================================================================

// 01: rule 1013 - consecutive zero-load -> clr a / mov Rn,a
void test_1013_zero_pair(void) __naked {
    __asm
    mov  r0,#0x00
    mov  r1,#0x00
00011$:
    mov  0x80,r0
    mov  0x81,r1
    ret
    __endasm;
}

// 02: rule 1001 - cjne a,#0x01 / cpl c -> add a,#0xff
void test_1001_bool_test(void) __naked {
    __asm
    cjne a,#0x01,00021$
00021$:
    cpl  c
00022$:
    jc   00023$
    mov  0x80,r0
    ret
00023$:
    mov  0x81,r0
    ret
    __endasm;
}

// 03: rule 1009 - dead zero-extend OR
void test_1009_zero_extend(void) __naked {
    __asm
    mov  r5,#0x00
    orl  ar6,a
    mov  a,r5
    orl  ar7,a
00031$:
    mov  0x80,r6
    ret
    __endasm;
}

// 04: rule 1010 - bit-mask OR merge
// DISABLED: upstream Peephole 302 removes mov r7,a (r7 dead after orl),
// but rule 1010 requires notUsed(r7). Cannot test in isolation.
//void test_1010_bitmask_merge(void) __naked {
//    __asm
//    anl  a,#0x0f
//    mov  r7,a
//    mov  a,r6
//    orl  a,r7
//00041$:
//    mov  0x80,a
//    ret
//    __endasm;
//}


// 05: rule 1008 - ones complement 0xFF - x -> cpl
void test_1008_complement(void) __naked {
    __asm
    mov  a,#0xff
    clr  c
    subb a,r7
00051$:
    mov  0x80,a
    ret
    __endasm;
}

// 06: rule 1013 fires before _Noreturn call (A dead - no return)
// DISABLED: upstream Peephole 302 removes mov r0,#0x00 before our rule fires.
// Upstream sees _Noreturn -> r0 dead -> removes. Cannot test rule 1013 in isolation.
//void test_noreturn_acc_dead(void) __naked {
//    __asm
//    mov  r0,#0x00
//    mov  0x80,r0
//00061$:
//    lcall _boot_vec
//    ret
//    __endasm;
//}

// =====================================================================
// MUST NOT FIRE - rule must not optimize
// =====================================================================

// 07: rule 1013 must not fire - A is live after
void test_1013_acc_live(void) __naked {
    __asm
    mov  r0,#0x00
    mov  r1,#0x00
00071$:
    mov  0x80,a
    ret
    __endasm;
}

// 08: rule 1008 must not fire - carry is live after subb
void test_1008_carry_live(void) __naked {
    __asm
    mov  a,#0xff
    clr  c
    subb a,r7
00081$:
    jc   00082$
    mov  0x80,a
    ret
00082$:
    mov  0x81,a
    ret
    __endasm;
}

// 09: rule 1013 must not fire before literal call (no symbol info, A unknown)
void test_literal_call_acc_unknown(void) __naked {
    __asm
    mov  r0,#0x00
00091$:
    lcall 0x3800
    ret
    __endasm;
}

// =====================================================================
// Plain C tests (no __naked, no inline asm)
// SDCC codegen produces patterns naturally, peephole optimizes them.
// =====================================================================

#include <stdint.h>
volatile __xdata uint8_t xbuf[8];

// 10: rule 1013 - zeroing multiple xdata bytes
void test_1013_plain(void) {
    xbuf[0] = 0;
    xbuf[1] = 0;
    xbuf[2] = 0;
    xbuf[3] = 0;
    xbuf[4] = 0;
    xbuf[5] = 0;
}

// 11: rule 1012 - sequential xdata writes to adjacent addresses
void test_1012_plain(void) {
    xbuf[0] = 0x41;
    xbuf[1] = 0x42;
    xbuf[2] = 0x43;
    xbuf[3] = 0x44;
}

// sentinel
void test_end(void) __naked { __asm ret __endasm; }
