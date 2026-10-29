# SPDX-License-Identifier: MIT
# Milestone lm-stateup (lane A2): Luminary099 FRESH_START_AND_RESTART bank 13 (STATEUP, THISVINT)
# as interpretive routines of the control-transfer ABI.
LM_STATEUP_SRC = src/lm_stateup.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_stateup_cases.c
LM_STATEUP_HDR = include/lm_stateup.h include/lm_interp_steps.h include/lm_interp_orders.h include/agc_transfer.h include/agc_image_ops.h tests/lm_stateup_cases.h
.PHONY: test-lm-stateup sanitize-lm-stateup c64-lm-stateup vice-lm-stateup lm-stateup-differential
all: build/test-lm-stateup
test: test-lm-stateup
sanitize: sanitize-lm-stateup
build/test-lm-stateup: $(LM_STATEUP_SRC) tests/test_lm_stateup.c tests/lm_stateup_vectors.h tests/lm_stateup_c64_vectors.h $(LM_STATEUP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_STATEUP_SRC) tests/test_lm_stateup.c -o $@
test-lm-stateup: build/test-lm-stateup
	./build/test-lm-stateup
sanitize-lm-stateup: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_STATEUP_SRC) tests/test_lm_stateup.c -o build/test-lm-stateup-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-stateup-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-stateup-native: $(LM_STATEUP_SRC) platform/host/lm_stateup_snapshot.c $(LM_STATEUP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_STATEUP_SRC) platform/host/lm_stateup_snapshot.c -o $@
lm-stateup-differential: build/lm-stateup-native
	python3 tools/lm_stateup_freeze.py
	git diff --exit-code -- tests/lm_stateup_vectors.h tests/lm_stateup_c64_vectors.h
c64-lm-stateup: build/lm-stateup.prg
build/lm-stateup.prg: $(LM_STATEUP_SRC) platform/c64/lm_stateup_diagnostic.c tests/lm_stateup_c64_vectors.h $(LM_STATEUP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-stateup.map $(LM_STATEUP_SRC) platform/c64/lm_stateup_diagnostic.c -o $@
vice-lm-stateup: build/lm-stateup.prg
	python3 tools/vice_run.py build/lm-stateup.prg --cycles 8000000000 --timeout 2400
