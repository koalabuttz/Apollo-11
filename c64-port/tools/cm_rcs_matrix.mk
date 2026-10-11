# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
RCS_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
RCS_CORE = src/cm_rcs_matrix.c src/cm_single_return.c src/cm_quictrig.c src/agc_word.c src/agc_multiword.c tests/cm_rcs_matrix_cases.c
all: build/cm-rcs-matrix-native build/cm-rcs-matrix-reference
build/cm-rcs-matrix-native: tools/cm_rcs_matrix_snapshot.c $(RCS_CORE)
	gcc -O2 $(RCS_FLAGS) tools/cm_rcs_matrix_snapshot.c $(RCS_CORE) -o $@
build/cm-rcs-matrix-reference: tools/cm_rcs_matrix_reference.c tests/cm_rcs_matrix_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/cm_rcs_matrix_reference.c tests/cm_rcs_matrix_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/cm-rcs-matrix-reference deps/cm-rcs-matrix-fixture/MAIN.agc.bin | gzip -n -9 > build/cm-rcs-matrix-reference.snapshots.gz
	build/cm-rcs-matrix-native | gzip -n -9 > build/cm-rcs-matrix-native.snapshots.gz
	cmp build/cm-rcs-matrix-reference.snapshots.gz build/cm-rcs-matrix-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(RCS_FLAGS) tools/cm_rcs_matrix_snapshot.c $(RCS_CORE) -o build/cm-rcs-matrix-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-rcs-matrix-sanitize | gzip -n -9 > build/cm-rcs-matrix-sanitize.snapshots.gz
	cmp build/cm-rcs-matrix-reference.snapshots.gz build/cm-rcs-matrix-sanitize.snapshots.gz
.PHONY: all test sanitize
check-host:
	gcc -O2 $(RCS_FLAGS) tests/test_cm_rcs_matrix.c tests/frozen_words.c $(RCS_CORE) -o build/test-cm-rcs-matrix
	build/test-cm-rcs-matrix
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DCM_RCS_MATRIX_CHECK_ONLY $(RCS_FLAGS) tests/test_cm_rcs_matrix.c tests/frozen_words.c tools/cm_rcs_matrix_diagnostic.c $(RCS_CORE) -Wl,-Map,build/cm-rcs-matrix0.map -o build/cm-rcs-matrix0.prg
vice-test: c64
	python tools/cm_rcs_matrix_vice_test.py --phase 0 --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: check-host c64 vice-test
