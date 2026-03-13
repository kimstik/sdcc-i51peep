# Peephole notUsed testbench for MCS51
#
# Usage:
#   make SDCC_HOME=/path/to/sdcc    - run all tests (27 total)
#   make compile                     - compile only
#   make clean                       - remove generated files
#
# 11 positive tests (rule SHOULD fire) + 16 negative tests (MUST NOT fire).
# Results verified via {peep:N} markers in output assembly.

CC = $(SDCC_HOME)/bin/sdcc
CFLAGS = -mmcs51 --peep-asm --fverbose-asm
BUILD = tmp

.PHONY: all compile clean

all: $(BUILD)/tests.asm
	@bash check.sh $<

compile: $(BUILD)/tests.asm

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/tests.asm: tests.c tests.def | $(BUILD)
	$(CC) $(CFLAGS) --peep-file tests.def -S -o $@ $<

clean:
	rm -rf $(BUILD)
