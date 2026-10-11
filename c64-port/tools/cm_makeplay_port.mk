# SPDX-License-Identifier: MIT
include tools/cm_makeplay.mk
test: cm_makeplay-host
sanitize: sanitize-cm-makeplay-frozen
sanitize-cm-makeplay-frozen: | build
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMMPLAY_FLAGS) tests/test_cm_makeplay.c tests/cm_makeplay_check.c $(CMMPLAY_CORE) -o build/test-cm-makeplay-frozen-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-makeplay-frozen-sanitize
.PHONY: sanitize-cm-makeplay-frozen
