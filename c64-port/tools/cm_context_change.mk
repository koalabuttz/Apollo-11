# SPDX-License-Identifier: MIT
CM_CONTEXT_CHANGE_CORE = src/cm_context_change.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_context_change_cases.c tests/cm_admission_complete_cases.c
build/cm-context-change-native: $(CM_CONTEXT_CHANGE_CORE) platform/host/cm_context_change_snapshot.c include/cm_context_change.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_CONTEXT_CHANGE_CORE) platform/host/cm_context_change_snapshot.c -o $@
cm-context-change-differential: build/cm-context-change-native prepare-cm-original-rope
	python3 tools/cm_context_change_differential.py
.PHONY: test-cm-context-change sanitize-cm-context-change
test: test-cm-context-change
test-cm-context-change: cm-context-change-differential
sanitize: sanitize-cm-context-change
sanitize-cm-context-change: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_CONTEXT_CHANGE_CORE) platform/host/cm_context_change_snapshot.c -o build/cm-context-change-sanitize
	python3 tools/cm_context_change_sanitize.py
