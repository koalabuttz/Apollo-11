# SPDX-License-Identifier: MIT
# agc_fast_ops (lane F): fast agc_image_ops helpers for plain erasable cells
# with 6502 MP/DV cores, and their one-byte EBANK 6 window forms
# (agc_fast_window). Checked against agc_image_ops on random images
# (host and C64) and against the pinned yaAGC engine per instruction.
AFO_SRC = src/agc_fast_ops.c src/agc_fast_window.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_multiword.c src/agc_word.c
AFO_HDR = include/agc_fast_ops.h include/agc_fast_window.h include/agc_image_ops.h include/agc_hardware_divide.h
.PHONY: test-agc-fast-ops sanitize-agc-fast-ops c64-agc-fast-ops vice-agc-fast-ops agc-fast-ops-differential
all: build/test-agc-fast-ops
test: test-agc-fast-ops
sanitize: sanitize-agc-fast-ops
build/test-agc-fast-ops: $(AFO_SRC) $(AFO_HDR) tests/test_agc_fast_ops.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(AFO_SRC) tests/test_agc_fast_ops.c -o $@
test-agc-fast-ops: build/test-agc-fast-ops
	./build/test-agc-fast-ops
	python3 tools/agc_fast_ops_differential.py --cases 2000
sanitize-agc-fast-ops: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -DCASES=100000ul -DMULDIV_CASES=200000ul $(AFO_SRC) tests/test_agc_fast_ops.c -o build/test-agc-fast-ops-sanitize
	./build/test-agc-fast-ops-sanitize
agc-fast-ops-differential:
	python3 tools/agc_fast_ops_differential.py
c64-agc-fast-ops: build/agc-fast-ops.prg
build/agc-fast-ops.prg: $(AFO_SRC) $(AFO_HDR) tests/test_agc_fast_ops.c platform/c64/agc_fast_ops_diagnostic.c | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Oz -DRK_FIXED_IMAGE=1 -DAFO_CHECK_NO_MAIN -DAFO_CHECK_QUIET -Wall -Wextra -Werror -Wl,-Map,build/agc-fast-ops.map $(AFO_SRC) tests/test_agc_fast_ops.c platform/c64/agc_fast_ops_diagnostic.c -o $@
vice-agc-fast-ops: build/agc-fast-ops.prg
	python3 tools/vice_run.py build/agc-fast-ops.prg --cycles 3000000000 --timeout 1200
