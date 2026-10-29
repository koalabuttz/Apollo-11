# SPDX-License-Identifier: MIT
CM_ENTRYTASK_CORE = src/cm_entry_tasks.c src/cm_phase.c src/cm_native_image.c src/cm_waitlist.c src/cm_waitlist_native.c src/agc_word.c tests/cm_entry_tasks_cases.c
CM_ENTRYTASK_CHECK = tests/cm_entry_tasks_check.c
CM_ENTRYTASK_HEADERS = include/cm_entry_tasks.h include/cm_phase.h include/cm_native_image.h include/cm_waitlist_native.h include/cm_waitlist.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_entry_tasks_cases.h tests/cm_entry_tasks_check.h tests/cm_entry_tasks_vectors.h
.PHONY: cm-entry-tasks-differential test-cm-entry-tasks sanitize-cm-entry-tasks c64-cm-entry-tasks
all: build/cm-entry-tasks-native build/test-cm-entry-tasks
build/cm-entry-tasks-native: $(CM_ENTRYTASK_CORE) platform/host/cm_entry_tasks_snapshot.c $(CM_ENTRYTASK_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ENTRYTASK_CORE) platform/host/cm_entry_tasks_snapshot.c -o $@
cm-entry-tasks-differential: build/cm-entry-tasks-native
	python3 tools/cm_entry_tasks_differential.py
build/test-cm-entry-tasks: $(CM_ENTRYTASK_CORE) $(CM_ENTRYTASK_CHECK) tests/test_cm_entry_tasks.c $(CM_ENTRYTASK_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ENTRYTASK_CORE) $(CM_ENTRYTASK_CHECK) tests/test_cm_entry_tasks.c -o $@
test: test-cm-entry-tasks
test-cm-entry-tasks: build/test-cm-entry-tasks
	./build/test-cm-entry-tasks
sanitize: sanitize-cm-entry-tasks
sanitize-cm-entry-tasks: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ENTRYTASK_CORE) $(CM_ENTRYTASK_CHECK) tests/test_cm_entry_tasks.c -o build/test-cm-entry-tasks-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-entry-tasks-sanitize
c64-cm-entry-tasks: build/cm-entry-tasks.prg
build/cm-entry-tasks.prg: $(CM_ENTRYTASK_CORE) $(CM_ENTRYTASK_CHECK) platform/c64/cm_entry_tasks_diagnostic.c $(CM_ENTRYTASK_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-entry-tasks.map $(CM_ENTRYTASK_CORE) $(CM_ENTRYTASK_CHECK) platform/c64/cm_entry_tasks_diagnostic.c -o $@
