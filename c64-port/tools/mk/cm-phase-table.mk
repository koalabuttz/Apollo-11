# SPDX-License-Identifier: MIT
# Milestone cm-phase-table (lane A2): Comanche055 PHASE_TABLE_MAINTENANCE
# (NEWMODEX/MMDSPLAY/SETUPDSP, CHECKMM, PHASCHNG/2PHSCHNG) as native routines
# of the control-transfer ABI.
CM_PT_SRC = src/cm_phase_table.c src/agc_image_ops.c src/agc_hardware_divide.c src/cm_interbank_routines.c src/cm_interbank_complete.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_phase_table_cases.c
CM_PT_HDR = include/cm_phase_table.h include/agc_transfer.h include/agc_image_ops.h src/cm_phase_sites.inc tests/cm_phase_table_cases.h
.PHONY: test-cm-phase-table sanitize-cm-phase-table c64-cm-phase-table vice-cm-phase-table cm-phase-table-differential
all: build/test-cm-phase-table
test: test-cm-phase-table
sanitize: sanitize-cm-phase-table
build/test-cm-phase-table: $(CM_PT_SRC) tests/test_cm_phase_table.c tests/cm_phase_table_vectors.h $(CM_PT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_PT_SRC) tests/test_cm_phase_table.c -o $@
test-cm-phase-table: build/test-cm-phase-table
	python3 tools/cm_phase_sites_generate.py --check
	./build/test-cm-phase-table
sanitize-cm-phase-table: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_PT_SRC) tests/test_cm_phase_table.c -o build/test-cm-phase-table-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-phase-table-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-phase-table-native: $(CM_PT_SRC) platform/host/cm_phase_table_snapshot.c $(CM_PT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_PT_SRC) platform/host/cm_phase_table_snapshot.c -o $@
cm-phase-table-differential: build/cm-phase-table-native
	python3 tools/cm_phase_table_freeze.py
	git diff --exit-code -- tests/cm_phase_table_vectors.h tests/cm_phase_table_c64_vectors.h
c64-cm-phase-table: build/cm-phase-table.prg
build/cm-phase-table.prg: $(CM_PT_SRC) platform/c64/cm_phase_table_diagnostic.c tests/cm_phase_table_c64_vectors.h $(CM_PT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-phase-table.map $(CM_PT_SRC) platform/c64/cm_phase_table_diagnostic.c -o $@
vice-cm-phase-table: build/cm-phase-table.prg
	python3 tools/vice_run.py build/cm-phase-table.prg --cycles 8000000000 --timeout 2400
