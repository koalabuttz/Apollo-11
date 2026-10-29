# SPDX-License-Identifier: MIT
CM_PRIORITY_DISPLAY_CORE = src/cm_priority_display.c src/cm_admission_zero.c src/cm_native_image.c src/cm_executive.c src/agc_word.c tests/cm_priority_display_cases.c tests/cm_admission_complete_cases.c
build/cm-priority-display-native: $(CM_PRIORITY_DISPLAY_CORE) platform/host/cm_priority_display_snapshot.c include/cm_priority_display.h | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_PRIORITY_DISPLAY_CORE) platform/host/cm_priority_display_snapshot.c -o $@
cm-priority-display-differential: build/cm-priority-display-native prepare-cm-original-rope
	python3 tools/cm_priority_display_differential.py
.PHONY: test-cm-priority-display sanitize-cm-priority-display
test: test-cm-priority-display
test-cm-priority-display: cm-priority-display-differential
sanitize: sanitize-cm-priority-display
sanitize-cm-priority-display: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_PRIORITY_DISPLAY_CORE) platform/host/cm_priority_display_snapshot.c -o build/cm-priority-display-sanitize
	python3 tools/cm_priority_display_sanitize.py
