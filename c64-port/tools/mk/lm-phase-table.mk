# SPDX-License-Identifier: MIT
# Milestone lm-phase-table (lane A2): Luminary099 PHASE_TABLE_MAINTENANCE
# (NEWMODEX/MMDSPLAY/SETUPDSP, CHECKMM, PHASCHNG/2PHSCHNG/PHSCHNGA) as native routines
# of the control-transfer ABI.
LM_PT_SRC = src/lm_phase_table.c src/agc_image_ops.c src/agc_hardware_divide.c src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_phase_table_cases.c
LM_PT_HDR = include/lm_phase_table.h include/lm_interbank_routines.h include/agc_transfer.h include/agc_image_ops.h src/lm_phase_sites.inc tests/lm_phase_table_cases.h
.PHONY: test-lm-phase-table sanitize-lm-phase-table c64-lm-phase-table vice-lm-phase-table lm-phase-table-differential
all: build/test-lm-phase-table
test: test-lm-phase-table
sanitize: sanitize-lm-phase-table
build/test-lm-phase-table: $(LM_PT_SRC) tests/test_lm_phase_table.c tests/lm_phase_table_vectors.h $(LM_PT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_PT_SRC) tests/test_lm_phase_table.c -o $@
test-lm-phase-table: build/test-lm-phase-table
	python3 tools/lm_phase_sites_generate.py --check
	./build/test-lm-phase-table
sanitize-lm-phase-table: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_PT_SRC) tests/test_lm_phase_table.c -o build/test-lm-phase-table-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-phase-table-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-phase-table-native: $(LM_PT_SRC) platform/host/lm_phase_table_snapshot.c $(LM_PT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_PT_SRC) platform/host/lm_phase_table_snapshot.c -o $@
lm-phase-table-differential: build/lm-phase-table-native
	python3 tools/lm_phase_table_freeze.py
	git diff --exit-code -- tests/lm_phase_table_vectors.h tests/lm_phase_table_c64_vectors.h
c64-lm-phase-table: build/lm-phase-table.prg
build/lm-phase-table.prg: $(LM_PT_SRC) platform/c64/lm_phase_table_diagnostic.c tests/lm_phase_table_c64_vectors.h $(LM_PT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-phase-table.map $(LM_PT_SRC) platform/c64/lm_phase_table_diagnostic.c -o $@
vice-lm-phase-table: build/lm-phase-table.prg
	python3 tools/vice_run.py build/lm-phase-table.prg --cycles 8000000000 --timeout 2400
