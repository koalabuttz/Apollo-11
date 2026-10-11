# SPDX-License-Identifier: MIT
CC ?= cc
EQ_FLAGS=-std=c99 -Wall -Wextra -Werror -Iinclude -Itests -ffunction-sections -fdata-sections -Wl,--gc-sections -DPREPARE_INPUT_ONLY -DSCALE_INPUT_ONLY -DPUSH_INPUT_ONLY -DELAPSED_INPUT_ONLY
EQ_SOURCES=src/agc_word.c src/agc_multiword.c src/p66_elapsed_return_control.c tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/elapsed_return_control_cases.c
.PHONY: test sanitize reference
reference:
	python3 tools/elapsed_return_control_differential.py
test:
	$(CC) $(EQ_FLAGS) -O2 $(EQ_SOURCES) tests/test_elapsed_return_control.c -o build/test-elapsed-return-control
	build/test-elapsed-return-control
	python3 tools/elapsed_return_control_differential.py --retained
sanitize:
	$(CC) $(EQ_FLAGS) -O1 -g -no-pie -fsanitize=address,undefined $(EQ_SOURCES) tests/test_elapsed_return_control.c -o build/test-elapsed-return-control-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-elapsed-return-control-sanitize
	python3 tools/elapsed_return_control_differential.py --retained --sanitize
