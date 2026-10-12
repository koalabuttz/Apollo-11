# SPDX-License-Identifier: MIT
# Milestone cm-stateup (lane A2): Comanche055 FRESH_START_AND_RESTART bank 13 (STATEUP, STATEND,
# THISVINT) and R00's CALL INTSTALL; EXIT
# as interpretive routines of the control-transfer ABI.
CM_STATEUP_SRC = src/cm_stateup.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_stateup_cases.c
CM_STATEUP_HDR = include/cm_stateup.h include/cm_interp.h src/cm_interp_names.h src/lm_interp_orders.c src/lm_interp_steps.c include/agc_transfer.h include/agc_image_ops.h tests/cm_stateup_cases.h
.PHONY: test-cm-stateup sanitize-cm-stateup c64-cm-stateup vice-cm-stateup cm-stateup-differential
all: build/test-cm-stateup
test: test-cm-stateup
sanitize: sanitize-cm-stateup
build/test-cm-stateup: $(CM_STATEUP_SRC) tests/test_cm_stateup.c tests/cm_stateup_vectors.h $(CM_STATEUP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_STATEUP_SRC) tests/test_cm_stateup.c -o $@
test-cm-stateup: build/test-cm-stateup
	./build/test-cm-stateup
sanitize-cm-stateup: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_STATEUP_SRC) tests/test_cm_stateup.c -o build/test-cm-stateup-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-stateup-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-stateup-native: $(CM_STATEUP_SRC) platform/host/cm_stateup_snapshot.c $(CM_STATEUP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_STATEUP_SRC) platform/host/cm_stateup_snapshot.c -o $@
cm-stateup-differential: build/cm-stateup-native
	python3 tools/cm_stateup_freeze.py
	git diff --exit-code -- tests/cm_stateup_vectors.h tests/cm_stateup_c64_vectors.h
c64-cm-stateup: build/cm-stateup.prg
build/cm-stateup.prg: $(CM_STATEUP_SRC) platform/c64/cm_stateup_diagnostic.c tests/cm_stateup_c64_vectors.h $(CM_STATEUP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-stateup.map $(CM_STATEUP_SRC) platform/c64/cm_stateup_diagnostic.c -o $@
vice-cm-stateup: build/cm-stateup.prg
	python3 tools/vice_run.py build/cm-stateup.prg --cycles 8000000000 --timeout 2400
