# SPDX-License-Identifier: MIT
CM_SCAN_ZERO_CORE = src/cm_scan_zero.c src/cm_basic_finish.c src/cm_basic_sleep.c src/cm_priority_complete.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_scan_zero_cases.c
build/cm-scan-zero-native: $(CM_SCAN_ZERO_CORE) platform/host/cm_scan_zero_snapshot.c include/cm_scan_zero.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_SCAN_ZERO_CORE) platform/host/cm_scan_zero_snapshot.c -o $@
.PHONY: test-cm-scan-zero sanitize-cm-scan-zero cm-scan-zero-differential
test: test-cm-scan-zero
test-cm-scan-zero: cm-scan-zero-differential
cm-scan-zero-differential: build/cm-scan-zero-native prepare-cm-original-rope
	python3 tools/cm_scan_zero_differential.py
sanitize: sanitize-cm-scan-zero
sanitize-cm-scan-zero: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_SCAN_ZERO_CORE) platform/host/cm_scan_zero_snapshot.c -o build/cm-scan-zero-sanitize
	python3 tools/cm_scan_zero_sanitize.py
