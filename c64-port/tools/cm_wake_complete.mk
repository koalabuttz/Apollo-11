# SPDX-License-Identifier: MIT
CM_WAKE_COMPLETE_CORE = src/cm_wake_complete.c src/agc_word.c tests/cm_wake_complete_cases.c
build/cm-wake-complete-native: $(CM_WAKE_COMPLETE_CORE) platform/host/cm_wake_complete_snapshot.c include/cm_wake_complete.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_WAKE_COMPLETE_CORE) platform/host/cm_wake_complete_snapshot.c -o $@
.PHONY: test-cm-wake-complete sanitize-cm-wake-complete cm-wake-complete-differential
test: test-cm-wake-complete
test-cm-wake-complete: cm-wake-complete-differential
cm-wake-complete-differential: build/cm-wake-complete-native prepare-cm-original-rope
	python3 tools/cm_wake_complete_differential.py
sanitize: sanitize-cm-wake-complete
sanitize-cm-wake-complete: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(CM_WAKE_COMPLETE_CORE) platform/host/cm_wake_complete_snapshot.c -o build/cm-wake-complete-sanitize
	python3 tools/cm_wake_complete_sanitize.py
