CM_RESTART_CORE = src/agc_word.c src/cm_executive.c src/cm_restart.c
CM_RESTART_INPUT = tests/cm_restart_cases.c
CM_RESTART_HEADERS = include/cm_restart.h include/cm_executive.h include/cm_waitlist.h tests/cm_restart_cases.h tests/cm_restart_check.h tests/cm_restart_vectors.h tests/cm_restart_addresses.h
all: build/cm-restart-snapshot build/test-cm-restart
build/cm-restart-snapshot: $(CM_RESTART_CORE) $(CM_RESTART_INPUT) platform/host/cm_restart_snapshot.c $(CM_RESTART_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RESTART_CORE) $(CM_RESTART_INPUT) platform/host/cm_restart_snapshot.c -o $@
build/test-cm-restart: $(CM_RESTART_CORE) $(CM_RESTART_INPUT) tests/cm_restart_check.c tests/test_cm_restart.c $(CM_RESTART_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RESTART_CORE) $(CM_RESTART_INPUT) tests/cm_restart_check.c tests/test_cm_restart.c -o $@
.PHONY: cm-restart-differential test-cm-restart sanitize-cm-restart c64-cm-restart
cm-restart-differential: build/cm-restart-snapshot
	python3 tools/cm_restart_differential.py
test: test-cm-restart
test-cm-restart: build/test-cm-restart
	./build/test-cm-restart
sanitize: sanitize-cm-restart
sanitize-cm-restart: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_RESTART_CORE) $(CM_RESTART_INPUT) tests/cm_restart_check.c tests/test_cm_restart.c -o build/test-cm-restart-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-restart-sanitize
c64-cm-restart: build/cm-restart.prg
build/cm-restart.prg: $(CM_RESTART_CORE) $(CM_RESTART_INPUT) tests/cm_restart_check.c platform/c64/cm_restart_diagnostic.c $(CM_RESTART_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-restart.map $(CM_RESTART_CORE) $(CM_RESTART_INPUT) tests/cm_restart_check.c platform/c64/cm_restart_diagnostic.c -o $@
