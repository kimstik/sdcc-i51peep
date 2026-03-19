#!/bin/bash
# check.sh - Verify peephole testbench results
# Usage: bash check.sh <tests.asm>
#
# Looks for "<rule" markers in each function's asm output.
# FIRE = rule must fire (marker present), KEEP = rule must not fire (no marker).

ASM=${1:?Usage: bash check.sh <tests.asm>}
pass=0 fail=0

check() {
    local fn=$1 expect=$2 desc=$3
    local found
    found=$(awk '/^_'"$fn"':/{p=1;next} /^_[a-z]/{p=0} p' "$ASM" | grep -c '<rule' || true)
    if [ "$expect" = "FIRE" ] && [ "$found" -gt 0 ]; then
        printf "PASS  FIRE  %-40s %s\n" "$fn" "$desc"
        ((pass++))
    elif [ "$expect" = "KEEP" ] && [ "$found" -eq 0 ]; then
        printf "PASS  KEEP  %-40s %s\n" "$fn" "$desc"
        ((pass++))
    else
        printf "FAIL  %-4s  %-40s %s (hits=%d)\n" "$expect" "$fn" "$desc" "$found"
        ((fail++))
    fi
}

echo "=== SHOULD FIRE ==="
check test_1013_zero_pair        FIRE "1013: consecutive zero-load"
check test_1001_bool_test        FIRE "1001: cjne/cpl -> add"
check test_1009_zero_extend      FIRE "1009: dead zero-extend OR"
check test_1008_complement       FIRE "1008: 0xFF-x -> cpl"
#check test_noreturn_acc_dead     FIRE "_Noreturn: A dead"  # disabled: upstream 302 removes pattern first

echo ""
echo "=== MUST NOT FIRE ==="
check test_1013_acc_live         KEEP "1013: A live, must not fire"
check test_1008_carry_live       KEEP "1008: CY live, must not fire"
check test_literal_call_acc_unknown KEEP "literal call: A unknown"

echo ""
echo "=== Plain C tests ==="
check test_1013_plain            FIRE "1013: plain C zero-load"
check test_1012_plain            FIRE "1012: plain C sequential DPTR"

echo ""
total=$((pass + fail))
echo "==============================="
echo "$pass/$total passed, $fail failed"
[ "$fail" -eq 0 ]
