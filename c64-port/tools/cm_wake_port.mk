# SPDX-License-Identifier: MIT
include tools/cm_wake.mk
test: cm_wake-host
sanitize: sanitize-cm-wake-frozen
sanitize-cm-wake-frozen: | build
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMWAKE_FLAGS) tests/test_cm_wake.c tests/cm_wake_check.c $(CMWAKE_CORE) -o build/test-cm-wake-frozen-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-wake-frozen-sanitize
.PHONY: sanitize-cm-wake-frozen
