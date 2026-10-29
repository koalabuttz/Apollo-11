# SPDX-License-Identifier: MIT
include tools/cm_clockjob.mk
test: cm_clockjob-host
sanitize: sanitize-cm-clockjob-frozen
sanitize-cm-clockjob-frozen: | build
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMCLK_FLAGS) tests/test_cm_clockjob.c tests/cm_clockjob_check.c $(CMCLK_CORE) -o build/test-cm-clockjob-frozen-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-clockjob-frozen-sanitize
.PHONY: sanitize-cm-clockjob-frozen
