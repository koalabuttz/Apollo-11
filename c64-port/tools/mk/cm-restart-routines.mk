# SPDX-License-Identifier: MIT
# Milestone cm-restart-routines (lane A2): Comanche055 RESTARTS_ROUTINE and
# RESTART_TABLES (tables and NEWPHASE) as native routines of the
# control-transfer ABI.
CM_RR_SRC = src/cm_restart_routines.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_restart_routines_cases.c
CM_RR_HDR = include/cm_restart_routines.h include/agc_transfer.h src/cm_restart_table_data.inc tests/cm_restart_routines_cases.h
.PHONY: test-cm-restart-routines sanitize-cm-restart-routines c64-cm-restart-routines vice-cm-restart-routines cm-restart-routines-differential
all: build/test-cm-restart-routines
test: test-cm-restart-routines
sanitize: sanitize-cm-restart-routines
build/test-cm-restart-routines: $(CM_RR_SRC) tests/test_cm_restart_routines.c tests/cm_restart_routines_vectors.h $(CM_RR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RR_SRC) tests/test_cm_restart_routines.c -o $@
test-cm-restart-routines: build/test-cm-restart-routines
	python3 tools/cm_restart_tables_generate.py --check
	./build/test-cm-restart-routines
sanitize-cm-restart-routines: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_RR_SRC) tests/test_cm_restart_routines.c -o build/test-cm-restart-routines-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-restart-routines-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-restart-routines-native: $(CM_RR_SRC) platform/host/cm_restart_routines_snapshot.c $(CM_RR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RR_SRC) platform/host/cm_restart_routines_snapshot.c -o $@
cm-restart-routines-differential: build/cm-restart-routines-native
	python3 tools/cm_restart_routines_freeze.py
	git diff --exit-code -- tests/cm_restart_routines_vectors.h tests/cm_restart_routines_c64_vectors.h
c64-cm-restart-routines: build/cm-restart-routines.prg
build/cm-restart-routines.prg: $(CM_RR_SRC) platform/c64/cm_restart_routines_diagnostic.c tests/cm_restart_routines_c64_vectors.h $(CM_RR_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-restart-routines.map $(CM_RR_SRC) platform/c64/cm_restart_routines_diagnostic.c -o $@
vice-cm-restart-routines: build/cm-restart-routines.prg
	python3 tools/vice_run.py build/cm-restart-routines.prg --cycles 8000000000 --timeout 2400
