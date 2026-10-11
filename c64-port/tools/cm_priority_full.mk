# SPDX-License-Identifier: MIT
CM_PRIORITY_FULL_CORE = src/cm_priority_full.c src/cm_native_image.c src/cm_executive.c src/agc_word.c tests/cm_priority_full_cases.c tests/cm_context_change_cases.c tests/cm_admission_complete_cases.c tests/cm_executive_cases.c
build/cm-priority-full-native: $(CM_PRIORITY_FULL_CORE) platform/host/cm_priority_full_snapshot.c include/cm_priority_full.h tests/cm_priority_full_cases.h tools/cm_priority_full_dump.h tools/cm_alarm2_image_dump.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_PRIORITY_FULL_CORE) platform/host/cm_priority_full_snapshot.c -o $@
build/test-cm-priority-full: $(CM_PRIORITY_FULL_CORE) tests/test_cm_priority_full.c include/cm_priority_full.h tests/cm_priority_full_cases.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_PRIORITY_FULL_CORE) tests/test_cm_priority_full.c -o $@
.PHONY: test-cm-priority-full sanitize-cm-priority-full cm-priority-full-differential
test: test-cm-priority-full
test-cm-priority-full: cm-priority-full-differential build/test-cm-priority-full
	./build/test-cm-priority-full
cm-priority-full-differential: build/cm-priority-full-native prepare-cm-original-rope
	python3 tools/cm_priority_full_differential.py
sanitize: sanitize-cm-priority-full
sanitize-cm-priority-full: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(CM_PRIORITY_FULL_CORE) platform/host/cm_priority_full_snapshot.c -o build/cm-priority-full-sanitize
	python3 tools/cm_priority_full_sanitize.py
