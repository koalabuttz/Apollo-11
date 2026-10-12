# SPDX-License-Identifier: MIT
# Milestone lm-fresh-start (lane A2): Luminary099 FRESH_START_AND_RESTART bank 05 and GOTOPOOH (fresh start,
# restart, ENEMA) as native routines of the control-transfer ABI.
LM_FS_SRC = src/lm_fresh_start.c src/agc_image_ops.c src/agc_hardware_divide.c src/lm_interbank_routines.c src/lm_interbank.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_fresh_start_cases.c
LM_FS_HDR = include/lm_fresh_start.h include/lm_interbank_routines.h include/agc_transfer.h include/agc_image_ops.h src/lm_fresh_start_operands.inc tests/lm_fresh_start_cases.h
.PHONY: test-lm-fresh-start sanitize-lm-fresh-start c64-lm-fresh-start vice-lm-fresh-start lm-fresh-start-differential
all: build/test-lm-fresh-start
test: test-lm-fresh-start
sanitize: sanitize-lm-fresh-start
build/test-lm-fresh-start: $(LM_FS_SRC) tests/test_lm_fresh_start.c tests/lm_fresh_start_vectors.h $(LM_FS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_FS_SRC) tests/test_lm_fresh_start.c -o $@
test-lm-fresh-start: build/test-lm-fresh-start
	python3 tools/file_operands_generate.py lm-fresh-start --check
	./build/test-lm-fresh-start
sanitize-lm-fresh-start: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_FS_SRC) tests/test_lm_fresh_start.c -o build/test-lm-fresh-start-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-fresh-start-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-fresh-start-native: $(LM_FS_SRC) platform/host/lm_fresh_start_snapshot.c $(LM_FS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_FS_SRC) platform/host/lm_fresh_start_snapshot.c -o $@
lm-fresh-start-differential: build/lm-fresh-start-native
	python3 tools/lm_fresh_start_freeze.py
	git diff --exit-code -- tests/lm_fresh_start_vectors.h tests/lm_fresh_start_c64_vectors.h
c64-lm-fresh-start: build/lm-fresh-start.prg
build/lm-fresh-start.prg: $(LM_FS_SRC) platform/c64/lm_fresh_start_diagnostic.c tests/lm_fresh_start_c64_vectors.h $(LM_FS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-fresh-start.map $(LM_FS_SRC) platform/c64/lm_fresh_start_diagnostic.c -o $@
vice-lm-fresh-start: build/lm-fresh-start.prg
	python3 tools/vice_run.py build/lm-fresh-start.prg --cycles 8000000000 --timeout 2400
