# SPDX-License-Identifier: MIT
CM_IDLE_CORE = src/cm_idle_dispatch.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_idle_dispatch_cases.c tests/cm_basic_finish_cases.c
CM_IDLE_HEADERS = include/cm_idle_dispatch.h include/cm_dispatch.h include/cm_executive.h include/cm_native_image.h include/overlay_runtime.h tests/cm_idle_dispatch_cases.h tests/cm_idle_dispatch_check.h tests/cm_idle_dispatch_vectors.h
all: build/cm-idle-dispatch-native
build/cm-idle-dispatch-native: $(CM_IDLE_CORE) platform/host/cm_idle_dispatch_snapshot.c $(CM_IDLE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IDLE_CORE) platform/host/cm_idle_dispatch_snapshot.c -o $@
cm-idle-dispatch-differential: build/cm-idle-dispatch-native
	python3 tools/cm_idle_dispatch_differential.py

CM_IDLE_CHECK = tests/cm_idle_dispatch_check.c
.PHONY: test-cm-idle-dispatch sanitize-cm-idle-dispatch c64-cm-idle-dispatch
test: test-cm-idle-dispatch
test-cm-idle-dispatch: build/test-cm-idle-dispatch
	./build/test-cm-idle-dispatch
build/test-cm-idle-dispatch: $(CM_IDLE_CORE) $(CM_IDLE_CHECK) tests/test_cm_idle_dispatch.c $(CM_IDLE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IDLE_CORE) $(CM_IDLE_CHECK) tests/test_cm_idle_dispatch.c -o $@
sanitize: sanitize-cm-idle-dispatch
sanitize-cm-idle-dispatch: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_IDLE_CORE) $(CM_IDLE_CHECK) tests/test_cm_idle_dispatch.c -o build/test-cm-idle-dispatch-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-idle-dispatch-sanitize
c64-cm-idle-dispatch: build/cm-idle-dispatch.prg
build/cm-idle-dispatch.prg: $(CM_IDLE_CORE) $(CM_IDLE_CHECK) platform/c64/cm_idle_dispatch_diagnostic.c $(CM_IDLE_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-idle-dispatch.map $(CM_IDLE_CORE) $(CM_IDLE_CHECK) platform/c64/cm_idle_dispatch_diagnostic.c -o $@
