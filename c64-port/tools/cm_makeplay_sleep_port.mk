# SPDX-License-Identifier: MIT
include tools/cm_makeplay_sleep.mk
test: cm_makeplay_sleep-host
sanitize: sanitize-cm-makeplay-sleep-frozen
sanitize-cm-makeplay-sleep-frozen: | build
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMMSLEEP_FLAGS) tests/test_cm_makeplay_sleep.c tests/cm_makeplay_sleep_check.c $(CMMSLEEP_CORE) -o build/test-cm-makeplay-sleep-frozen-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-makeplay-sleep-frozen-sanitize
.PHONY: sanitize-cm-makeplay-sleep-frozen
