# SPDX-License-Identifier: MIT
# Milestone cm-update (lane A2): Comanche055 UPDATE_PROGRAM (P27: V70-V73
# updates) as native routines of the control-transfer ABI.
CM_UP_SRC = src/cm_update.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/cm_interbank_routines.c src/cm_interbank_complete.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_update_cases.c
CM_UP_HDR = include/cm_update.h include/cm_interp.h src/cm_interp_names.h src/lm_interp_orders.c src/lm_interp_steps.c include/cm_interbank_routines.h include/cm_interbank_complete.h include/agc_transfer.h include/agc_image_ops.h src/cm_update_operands.inc tests/cm_update_cases.h
.PHONY: test-cm-update sanitize-cm-update c64-cm-update vice-cm-update cm-update-differential
all: build/test-cm-update
test: test-cm-update
sanitize: sanitize-cm-update
build/test-cm-update: $(CM_UP_SRC) tests/test_cm_update.c tests/cm_update_vectors.h tests/cm_update_c64_vectors.h $(CM_UP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_UP_SRC) tests/test_cm_update.c -o $@
test-cm-update: build/test-cm-update
	python3 tools/file_operands_generate.py cm-update --check
	./build/test-cm-update
sanitize-cm-update: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_UP_SRC) tests/test_cm_update.c -o build/test-cm-update-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-update-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-update-native: $(CM_UP_SRC) platform/host/cm_update_snapshot.c $(CM_UP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_UP_SRC) platform/host/cm_update_snapshot.c -o $@
cm-update-differential: build/cm-update-native
	python3 tools/cm_update_freeze.py
	git diff --exit-code -- tests/cm_update_vectors.h tests/cm_update_c64_vectors.h
c64-cm-update: build/cm-update.prg
build/cm-update.prg: $(CM_UP_SRC) platform/c64/cm_update_diagnostic.c tests/cm_update_c64_vectors.h $(CM_UP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-update.map $(CM_UP_SRC) platform/c64/cm_update_diagnostic.c -o $@
vice-cm-update: build/cm-update.prg
	python3 tools/vice_run.py build/cm-update.prg --cycles 8000000000 --timeout 2400
