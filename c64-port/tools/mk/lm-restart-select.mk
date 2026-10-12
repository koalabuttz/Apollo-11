# SPDX-License-Identifier: MIT
# Milestone lm-restart-select (lane A): Luminary099 RESTARTS_ROUTINE selecting
# RESTART_TABLES entries, as native routines of the control-transfer ABI.
LM_RS_SRC = src/lm_restart_select.c src/lm_restart_routines.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_restart_select_cases.c tests/lm_restart_routines_cases.c
LM_RS_HDR = include/lm_restart_select.h include/lm_restart_routines.h include/agc_transfer.h src/lm_restart_table_data.inc tests/lm_restart_select_cases.h tests/lm_restart_routines_cases.h
.PHONY: test-lm-restart-select sanitize-lm-restart-select c64-lm-restart-select vice-lm-restart-select lm-restart-select-differential
all: build/test-lm-restart-select
test: test-lm-restart-select
sanitize: sanitize-lm-restart-select
build/test-lm-restart-select: $(LM_RS_SRC) tests/test_lm_restart_routines.c tests/lm_restart_vectors.h $(LM_RS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_RS_SRC) tests/test_lm_restart_routines.c -o $@
test-lm-restart-select: build/test-lm-restart-select
	python3 tools/lm_restart_tables_generate.py --check
	./build/test-lm-restart-select
sanitize-lm-restart-select: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_RS_SRC) tests/test_lm_restart_routines.c -o build/test-lm-restart-select-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-restart-select-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-restart-select-native: $(LM_RS_SRC) platform/host/lm_restart_select_snapshot.c $(LM_RS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_RS_SRC) platform/host/lm_restart_select_snapshot.c -o $@
build/lm-restart-routines-native: $(LM_RS_SRC) platform/host/lm_restart_routines_snapshot.c $(LM_RS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_RS_SRC) platform/host/lm_restart_routines_snapshot.c -o $@
lm-restart-select-differential: build/lm-restart-select-native build/lm-restart-routines-native
	python3 tools/lm_restart_freeze.py
	git diff --exit-code -- tests/lm_restart_vectors.h tests/lm_restart_c64_vectors.h
c64-lm-restart-select: build/lm-restart-select.prg
build/lm-restart-select.prg: $(LM_RS_SRC) platform/c64/lm_restart_routines_diagnostic.c tests/lm_restart_c64_vectors.h $(LM_RS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-restart-select.map $(LM_RS_SRC) platform/c64/lm_restart_routines_diagnostic.c -o $@
vice-lm-restart-select: build/lm-restart-select.prg
	python3 tools/vice_run.py build/lm-restart-select.prg --cycles 8000000000 --timeout 2400
