# SPDX-License-Identifier: MIT
CM_RSSELECT_CORE = src/cm_restart_select.c src/cm_restart_timing.c src/agc_word.c src/agc_multiword.c tests/cm_restart_select_cases.c
CM_RSSELECT_CHECK = tests/cm_restart_select_check.c
CM_RSSELECT_HEADERS = include/cm_restart_select.h include/cm_restart_timing.h include/cm_restart_table_data.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_restart_select_cases.h tests/cm_restart_select_check.h tests/cm_restart_select_vectors.h
.PHONY: cm-restart-select-differential test-cm-restart-select sanitize-cm-restart-select c64-cm-restart-select
all: build/cm-restart-select-native build/test-cm-restart-select
build/cm-restart-select-native: $(CM_RSSELECT_CORE) platform/host/cm_restart_select_snapshot.c $(CM_RSSELECT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RSSELECT_CORE) platform/host/cm_restart_select_snapshot.c -o $@
cm-restart-select-differential: build/cm-restart-select-native
	python3 tools/cm_restart_select_differential.py
build/test-cm-restart-select: $(CM_RSSELECT_CORE) $(CM_RSSELECT_CHECK) tests/test_cm_restart_select.c $(CM_RSSELECT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RSSELECT_CORE) $(CM_RSSELECT_CHECK) tests/test_cm_restart_select.c -o $@
test: test-cm-restart-select
test-cm-restart-select: build/test-cm-restart-select
	./build/test-cm-restart-select
sanitize: sanitize-cm-restart-select
sanitize-cm-restart-select: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_RSSELECT_CORE) $(CM_RSSELECT_CHECK) tests/test_cm_restart_select.c -o build/test-cm-restart-select-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-restart-select-sanitize
c64-cm-restart-select: build/cm-restart-select.prg
build/cm-restart-select.prg: $(CM_RSSELECT_CORE) $(CM_RSSELECT_CHECK) platform/c64/cm_restart_select_diagnostic.c $(CM_RSSELECT_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-restart-select.map $(CM_RSSELECT_CORE) $(CM_RSSELECT_CHECK) platform/c64/cm_restart_select_diagnostic.c -o $@
