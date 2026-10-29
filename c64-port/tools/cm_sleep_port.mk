# SPDX-License-Identifier: MIT
include tools/cm_sleep.mk
test: cm_sleep-host
sanitize: sanitize-cm-sleep-frozen
sanitize-cm-sleep-frozen: | build
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMSLEEP_FLAGS) tests/test_cm_sleep.c tests/cm_sleep_check.c $(CMSLEEP_CORE) -o build/test-cm-sleep-frozen-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-sleep-frozen-sanitize
.PHONY: sanitize-cm-sleep-frozen
