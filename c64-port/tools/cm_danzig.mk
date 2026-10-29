# SPDX-License-Identifier: MIT
CM_DANZIG_CORE = src/cm_danzig.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_danzig_cases.c tests/cm_basic_finish_cases.c
CM_DANZIG_HEADERS = include/cm_danzig.h include/cm_dispatch.h include/cm_executive.h include/cm_native_image.h include/overlay_runtime.h tests/cm_danzig_cases.h tests/cm_danzig_check.h tests/cm_danzig_vectors.h
all: build/cm-danzig-native
build/cm-danzig-native: $(CM_DANZIG_CORE) platform/host/cm_danzig_snapshot.c $(CM_DANZIG_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DANZIG_CORE) platform/host/cm_danzig_snapshot.c -o $@
cm-danzig-differential: build/cm-danzig-native
	python3 tools/cm_danzig_differential.py

CM_DANZIG_CHECK = tests/cm_danzig_check.c
.PHONY: test-cm-danzig sanitize-cm-danzig c64-cm-danzig
test: test-cm-danzig
test-cm-danzig: build/test-cm-danzig
	./build/test-cm-danzig
build/test-cm-danzig: $(CM_DANZIG_CORE) $(CM_DANZIG_CHECK) tests/test_cm_danzig.c $(CM_DANZIG_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DANZIG_CORE) $(CM_DANZIG_CHECK) tests/test_cm_danzig.c -o $@
sanitize: sanitize-cm-danzig
sanitize-cm-danzig: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_DANZIG_CORE) $(CM_DANZIG_CHECK) tests/test_cm_danzig.c -o build/test-cm-danzig-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-danzig-sanitize
c64-cm-danzig: build/cm-danzig.prg
build/cm-danzig.prg: $(CM_DANZIG_CORE) $(CM_DANZIG_CHECK) platform/c64/cm_danzig_diagnostic.c $(CM_DANZIG_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-danzig.map $(CM_DANZIG_CORE) $(CM_DANZIG_CHECK) platform/c64/cm_danzig_diagnostic.c -o $@
