# SPDX-License-Identifier: MIT
# Milestone lm-update (lane A2): Luminary099 UPDATE_PROGRAM (P27: V70-V73
# updates) as native routines of the control-transfer ABI.
LM_UP_SRC = src/lm_update.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/lm_interbank_routines.c src/lm_interbank.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_update_cases.c
LM_UP_HDR = include/lm_update.h include/lm_interp_steps.h include/lm_interp_orders.h include/lm_interbank_routines.h include/agc_transfer.h include/agc_image_ops.h src/lm_update_operands.inc tests/lm_update_cases.h
.PHONY: test-lm-update sanitize-lm-update c64-lm-update vice-lm-update lm-update-differential
all: build/test-lm-update
test: test-lm-update
sanitize: sanitize-lm-update
build/test-lm-update: $(LM_UP_SRC) tests/test_lm_update.c tests/lm_update_vectors.h tests/lm_update_c64_vectors.h $(LM_UP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_UP_SRC) tests/test_lm_update.c -o $@
test-lm-update: build/test-lm-update
	python3 tools/file_operands_generate.py lm-update --check
	./build/test-lm-update
sanitize-lm-update: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_UP_SRC) tests/test_lm_update.c -o build/test-lm-update-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-update-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-update-native: $(LM_UP_SRC) platform/host/lm_update_snapshot.c $(LM_UP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_UP_SRC) platform/host/lm_update_snapshot.c -o $@
lm-update-differential: build/lm-update-native
	python3 tools/lm_update_freeze.py
	git diff --exit-code -- tests/lm_update_vectors.h tests/lm_update_c64_vectors.h
c64-lm-update: build/lm-update.prg
build/lm-update.prg: $(LM_UP_SRC) platform/c64/lm_update_diagnostic.c tests/lm_update_c64_vectors.h $(LM_UP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-update.map $(LM_UP_SRC) platform/c64/lm_update_diagnostic.c -o $@
vice-lm-update: build/lm-update.prg
	python3 tools/vice_run.py build/lm-update.prg --cycles 8000000000 --timeout 2400
