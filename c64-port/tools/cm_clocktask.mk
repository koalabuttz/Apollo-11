# SPDX-License-Identifier: MIT
CM_CLOCKTASK_CORE = src/cm_clocktask.c src/cm_phase.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c tests/cm_clocktask_cases.c
CM_CLOCKTASK_CHECK = tests/cm_clocktask_check.c
CM_CLOCKTASK_HEADERS = include/cm_clocktask.h include/cm_phase.h include/cm_native_image.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_clocktask_cases.h tests/cm_clocktask_check.h tests/cm_clocktask_vectors.h
.PHONY: cm-clocktask-differential test-cm-clocktask sanitize-cm-clocktask c64-cm-clocktask
all: build/cm-clocktask-native build/test-cm-clocktask
build/cm-clocktask-native: $(CM_CLOCKTASK_CORE) platform/host/cm_clocktask_snapshot.c $(CM_CLOCKTASK_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_CLOCKTASK_CORE) platform/host/cm_clocktask_snapshot.c -o $@
cm-clocktask-differential: build/cm-clocktask-native
	python3 tools/cm_clocktask_differential.py
build/test-cm-clocktask: $(CM_CLOCKTASK_CORE) $(CM_CLOCKTASK_CHECK) tests/test_cm_clocktask.c $(CM_CLOCKTASK_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_CLOCKTASK_CORE) $(CM_CLOCKTASK_CHECK) tests/test_cm_clocktask.c -o $@
test: test-cm-clocktask
test-cm-clocktask: build/test-cm-clocktask
	./build/test-cm-clocktask
sanitize: sanitize-cm-clocktask
sanitize-cm-clocktask: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_CLOCKTASK_CORE) $(CM_CLOCKTASK_CHECK) tests/test_cm_clocktask.c -o build/test-cm-clocktask-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-clocktask-sanitize
c64-cm-clocktask: build/cm-clocktask.prg
build/cm-clocktask.prg: $(CM_CLOCKTASK_CORE) $(CM_CLOCKTASK_CHECK) platform/c64/cm_clocktask_diagnostic.c $(CM_CLOCKTASK_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-clocktask.map $(CM_CLOCKTASK_CORE) $(CM_CLOCKTASK_CHECK) platform/c64/cm_clocktask_diagnostic.c -o $@
