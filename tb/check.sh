#!/bin/bash
# check.sh - Verify peephole notUsed testbench results
# Usage: bash check.sh tmp/tests.asm

ASM=${1:?Usage: bash check.sh <tests.asm>}
pass=0 fail=0

check() {
    local fn=$1 expect=$2 desc=$3
    # Extract function section: from _fn: to next _[a-z] label
    local found
    # nop is the universal replacement marker - no source function contains nop
    found=$(awk '/^_'"$fn"':/{p=1;next} /^_[a-z]/{p=0} p' "$ASM" | grep -c '	nop' || true)
    if [ "$expect" = "FIRE" ] && [ "$found" -gt 0 ]; then
        printf "PASS  FIRE  %-40s %s\n" "$fn" "$desc"
        ((pass++))
    elif [ "$expect" = "KEEP" ] && [ "$found" -eq 0 ]; then
        printf "PASS  KEEP  %-40s %s\n" "$fn" "$desc"
        ((pass++))
    else
        printf "FAIL  %-4s  %-40s %s (peep=%d)\n" "$expect" "$fn" "$desc" "$found"
        ((fail++))
    fi
}

echo "=== SHOULD FIRE (notUsed=TRUE, rule fires) ==="
check test_acc_basic                     FIRE "notUsed('a') basic"
check test_carry_dead_after_add          FIRE "add writes CY, dead"
check test_carry_not_blocked_by_ov       FIRE "jb ov != CY (PSW split)"
check test_carry_dead_at_lcall           FIRE "CY dead at lcall"
check test_carry_dead_after_cjne         FIRE "cjne writes CY, dead"
check test_carry_not_blocked_by_p        FIRE "jb p != CY (PSW split)"
check test_carry_killed_by_second_writer FIRE "2nd add kills 1st CY"
check test_carry_not_blocked_by_f0       FIRE "jb f0 != CY (PSW split)"
check test_carry_write_by_mov_c_bit      FIRE "mov c,bit writes CY"
check test_carry_dead_after_da           FIRE "da writes CY, dead"
check test_acc_dead_after_movx_read      FIRE "movx a,@dptr kills A"

echo ""
echo "=== MUST NOT FIRE (notUsed=FALSE, rule must not fire) ==="
check test_carry_strstr                  KEEP "addc reads CY (strstr)"
check test_carry_read_by_subb            KEEP "subb reads CY"
check test_carry_read_by_rlc             KEEP "rlc reads CY"
check test_carry_read_by_jc              KEEP "jc reads CY"
check test_carry_read_by_jnc             KEEP "jnc reads CY"
check test_carry_read_by_rrc             KEEP "rrc reads CY"
check test_carry_read_by_orl_c_bit       KEEP "orl c,bit reads CY"
check test_carry_read_by_anl_c_bit       KEEP "anl c,bit reads CY"
check test_carry_read_by_cpl_c           KEEP "cpl c reads CY"
check test_carry_read_by_push_psw        KEEP "push psw reads CY"
check test_acc_blocked_by_movx_write     KEEP "movx @dptr,a reads A"
check test_acc_blocked_by_mul            KEEP "mul ab reads A"
check test_acc_blocked_by_movc           KEEP "movc a,@a+dptr reads A"
check test_acc_blocked_by_xch            KEEP "xch a,r1 reads A"
check test_acc_blocked_by_push           KEEP "push acc reads A"
check test_carry_live_across_sjmp        KEEP "CY live across sjmp"

echo ""
total=$((pass + fail))
echo "==============================="
echo "$pass/$total passed, $fail failed"
[ "$fail" -eq 0 ]
