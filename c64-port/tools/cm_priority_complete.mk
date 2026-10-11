# SPDX-License-Identifier: MIT
CM_PRIORITY_COMPLETE_CORE = src/cm_priority_complete.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_priority_complete_cases.c tests/cm_context_change_cases.c tests/cm_admission_complete_cases.c
build/cm-priority-complete-native: $(CM_PRIORITY_COMPLETE_CORE) platform/host/cm_priority_complete_snapshot.c include/cm_priority_complete.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_PRIORITY_COMPLETE_CORE) platform/host/cm_priority_complete_snapshot.c -o $@
cm-priority-complete-differential: build/cm-priority-complete-native prepare-cm-original-rope
	python3 tools/cm_priority_complete_differential.py
.PHONY: test-cm-priority-complete sanitize-cm-priority-complete
test: test-cm-priority-complete
test-cm-priority-complete: cm-priority-complete-differential
sanitize: sanitize-cm-priority-complete
sanitize-cm-priority-complete: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_PRIORITY_COMPLETE_CORE) platform/host/cm_priority_complete_snapshot.c -o build/cm-priority-complete-sanitize
	python3 tools/cm_priority_complete_sanitize.py
