# SPDX-License-Identifier: MIT
CM_RSTIME_CORE = src/cm_restart_timing.c src/agc_word.c src/agc_multiword.c tests/cm_restart_timing_cases.c
CM_RSTIME_CHECK = tests/cm_restart_timing_check.c
CM_RSTIME_HEADERS = include/cm_restart_timing.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_restart_timing_cases.h tests/cm_restart_timing_check.h tests/cm_restart_timing_vectors.h
.PHONY: cm-restart-timing-differential test-cm-restart-timing sanitize-cm-restart-timing c64-cm-restart-timing
all: build/cm-restart-timing-native build/test-cm-restart-timing
build/cm-restart-timing-native: $(CM_RSTIME_CORE) platform/host/cm_restart_timing_snapshot.c $(CM_RSTIME_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RSTIME_CORE) platform/host/cm_restart_timing_snapshot.c -o $@
cm-restart-timing-differential: build/cm-restart-timing-native
	python3 tools/cm_restart_timing_differential.py
build/test-cm-restart-timing: $(CM_RSTIME_CORE) $(CM_RSTIME_CHECK) tests/test_cm_restart_timing.c $(CM_RSTIME_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RSTIME_CORE) $(CM_RSTIME_CHECK) tests/test_cm_restart_timing.c -o $@
test: test-cm-restart-timing
test-cm-restart-timing: build/test-cm-restart-timing
	./build/test-cm-restart-timing
sanitize: sanitize-cm-restart-timing
sanitize-cm-restart-timing: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_RSTIME_CORE) $(CM_RSTIME_CHECK) tests/test_cm_restart_timing.c -o build/test-cm-restart-timing-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-restart-timing-sanitize
c64-cm-restart-timing: build/cm-restart-timing.prg
build/cm-restart-timing.prg: $(CM_RSTIME_CORE) $(CM_RSTIME_CHECK) platform/c64/cm_restart_timing_diagnostic.c $(CM_RSTIME_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-restart-timing.map $(CM_RSTIME_CORE) $(CM_RSTIME_CHECK) platform/c64/cm_restart_timing_diagnostic.c -o $@
