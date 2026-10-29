CM_STARTUP_CORE = src/agc_word.c src/cm_executive.c src/cm_restart.c src/cm_native_image.c src/cm_startup.c
CM_STARTUP_INPUT = tests/cm_startup_cases.c
CM_STARTUP_HEADERS = include/cm_startup.h include/cm_native_image.h include/cm_restart.h include/cm_executive.h include/cm_waitlist.h include/cm_phase.h include/cm_enema.h include/overlay_runtime.h tests/cm_startup_cases.h tests/cm_startup_check.h tests/cm_startup_vectors.h
all: build/cm-startup-snapshot build/test-cm-startup
build/cm-startup-snapshot: $(CM_STARTUP_CORE) $(CM_STARTUP_INPUT) platform/host/cm_startup_snapshot.c $(CM_STARTUP_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_STARTUP_CORE) $(CM_STARTUP_INPUT) platform/host/cm_startup_snapshot.c -o $@
build/test-cm-startup: $(CM_STARTUP_CORE) $(CM_STARTUP_INPUT) tests/cm_startup_check.c tests/test_cm_startup.c $(CM_STARTUP_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_STARTUP_CORE) $(CM_STARTUP_INPUT) tests/cm_startup_check.c tests/test_cm_startup.c -o $@
.PHONY: cm-startup-differential test-cm-startup sanitize-cm-startup c64-cm-startup
cm-startup-differential: build/cm-startup-snapshot
	python3 tools/cm_startup_differential.py
test: test-cm-startup
test-cm-startup: build/test-cm-startup
	./build/test-cm-startup
sanitize: sanitize-cm-startup
sanitize-cm-startup: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_STARTUP_CORE) $(CM_STARTUP_INPUT) tests/cm_startup_check.c tests/test_cm_startup.c -o build/test-cm-startup-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-startup-sanitize
c64-cm-startup: build/cm-startup.prg
build/cm-startup.prg: $(CM_STARTUP_CORE) $(CM_STARTUP_INPUT) tests/cm_startup_check.c platform/c64/cm_startup_diagnostic.c $(CM_STARTUP_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-startup.map $(CM_STARTUP_CORE) $(CM_STARTUP_INPUT) tests/cm_startup_check.c platform/c64/cm_startup_diagnostic.c -o $@
