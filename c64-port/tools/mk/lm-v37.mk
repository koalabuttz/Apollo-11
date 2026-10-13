# SPDX-License-Identifier: MIT
# Milestone lm-v37 (lane A2): Luminary099 FRESH_START_AND_RESTART bank 04 (verb 37, R00, POOH, V37XEQ)
# as native routines of the control-transfer ABI.
LM_V37_SRC = src/lm_v37.c src/lm_fresh_start.c src/agc_image_ops.c src/agc_hardware_divide.c src/lm_interbank_routines.c src/lm_interbank.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_v37_cases.c
LM_V37_HDR = include/lm_v37.h include/lm_fresh_start.h include/lm_interbank_routines.h include/agc_transfer.h include/agc_image_ops.h src/lm_v37_operands.inc tests/lm_v37_cases.h
.PHONY: test-lm-v37 sanitize-lm-v37 c64-lm-v37 vice-lm-v37 lm-v37-differential
all: build/test-lm-v37
test: test-lm-v37
sanitize: sanitize-lm-v37
build/test-lm-v37: $(LM_V37_SRC) tests/test_lm_v37.c tests/lm_v37_vectors.h tests/lm_v37_c64_vectors.h $(LM_V37_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_V37_SRC) tests/test_lm_v37.c -o $@
test-lm-v37: build/test-lm-v37
	python3 tools/file_operands_generate.py lm-v37 --check
	./build/test-lm-v37
sanitize-lm-v37: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_V37_SRC) tests/test_lm_v37.c -o build/test-lm-v37-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-v37-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-v37-native: $(LM_V37_SRC) platform/host/lm_v37_snapshot.c $(LM_V37_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_V37_SRC) platform/host/lm_v37_snapshot.c -o $@
lm-v37-differential: build/lm-v37-native
	python3 tools/lm_v37_freeze.py
	git diff --exit-code -- tests/lm_v37_vectors.h tests/lm_v37_c64_vectors.h
# -Oz: banks 04 and 05 at -Os run past $c000, the mailbox.
c64-lm-v37: build/lm-v37.prg
build/lm-v37.prg: $(LM_V37_SRC) platform/c64/lm_v37_diagnostic.c tests/lm_v37_c64_vectors.h $(LM_V37_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-v37.map $(LM_V37_SRC) platform/c64/lm_v37_diagnostic.c -o $@
vice-lm-v37: build/lm-v37.prg
	python3 tools/vice_run.py build/lm-v37.prg --cycles 8000000000 --timeout 2400
