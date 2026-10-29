# SPDX-License-Identifier: MIT
include tools/cm_display_sleep_lifecycle.mk
test: cm_display_sleep_lifecycle-host
sanitize: sanitize-cm-display-sleep-lifecycle-frozen
sanitize-cm-display-sleep-lifecycle-frozen: | build
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMDISPLIF_FLAGS) tests/cm_display_sleep_lifecycle_host.c tests/cm_display_sleep_lifecycle_check.c $(CMDISPLIF_CORE) -o build/test-cm-display-sleep-lifecycle-frozen-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-display-sleep-lifecycle-frozen-sanitize
.PHONY: sanitize-cm-display-sleep-lifecycle-frozen
