CM_RN_CORE = src/agc_word.c src/cm_executive.c src/cm_dispatch.c src/cm_native_dispatch.c src/cm_restart.c src/cm_restart_native.c
CM_RN_INPUT = tests/cm_restart_cases.c tests/cm_restart_native_cases.c
CM_RN_HEADERS = include/cm_restart_native.h include/cm_restart.h include/cm_executive.h include/cm_waitlist.h include/cm_native_dispatch.h include/cm_dispatch.h tests/cm_restart_cases.h tests/cm_restart_native_cases.h tests/cm_restart_native_vectors.h tests/cm_restart_native_check.h
all: build/cm-restart-native-snapshot build/test-cm-restart-native
build/cm-restart-native-snapshot: $(CM_RN_CORE) $(CM_RN_INPUT) platform/host/cm_restart_native_snapshot.c $(CM_RN_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RN_CORE) $(CM_RN_INPUT) platform/host/cm_restart_native_snapshot.c -o $@
build/test-cm-restart-native: $(CM_RN_CORE) $(CM_RN_INPUT) tests/cm_restart_native_check.c tests/test_cm_restart_native.c $(CM_RN_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RN_CORE) $(CM_RN_INPUT) tests/cm_restart_native_check.c tests/test_cm_restart_native.c -o $@
.PHONY: test-cm-restart-native sanitize-cm-restart-native c64-cm-restart-native cm-restart-native-differential
test: test-cm-restart-native
test-cm-restart-native: build/test-cm-restart-native
	./build/test-cm-restart-native
sanitize: sanitize-cm-restart-native
sanitize-cm-restart-native: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_RN_CORE) $(CM_RN_INPUT) tests/cm_restart_native_check.c tests/test_cm_restart_native.c -o build/test-cm-restart-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-restart-native-sanitize
c64-cm-restart-native: build/cm-restart-native.prg
build/cm-restart-native.prg: $(CM_RN_CORE) $(CM_RN_INPUT) tests/cm_restart_native_check.c platform/c64/cm_restart_native_diagnostic.c $(CM_RN_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-restart-native.map $(CM_RN_CORE) $(CM_RN_INPUT) tests/cm_restart_native_check.c platform/c64/cm_restart_native_diagnostic.c -o $@
cm-restart-native-differential: build/cm-restart-native-snapshot
	python3 tools/cm_restart_native_differential.py
