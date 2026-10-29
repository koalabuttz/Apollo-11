# SPDX-License-Identifier: MIT
CM_SVCT3_CORE = src/cm_svct3.c src/cm_native_image.c src/agc_word.c tests/cm_svct3_cases.c
CM_SVCT3_CHECK = tests/cm_svct3_check.c
CM_SVCT3_HEADERS = include/cm_svct3.h include/cm_phase.h include/cm_native_image.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_svct3_cases.h tests/cm_svct3_check.h tests/cm_svct3_vectors.h
.PHONY: cm-svct3-differential test-cm-svct3 sanitize-cm-svct3 c64-cm-svct3
all: build/cm-svct3-native build/test-cm-svct3
build/cm-svct3-native: $(CM_SVCT3_CORE) platform/host/cm_svct3_snapshot.c $(CM_SVCT3_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_SVCT3_CORE) platform/host/cm_svct3_snapshot.c -o $@
cm-svct3-differential: build/cm-svct3-native
	python3 tools/cm_svct3_differential.py
build/test-cm-svct3: $(CM_SVCT3_CORE) $(CM_SVCT3_CHECK) tests/test_cm_svct3.c $(CM_SVCT3_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_SVCT3_CORE) $(CM_SVCT3_CHECK) tests/test_cm_svct3.c -o $@
test: test-cm-svct3
test-cm-svct3: build/test-cm-svct3
	./build/test-cm-svct3
sanitize: sanitize-cm-svct3
sanitize-cm-svct3: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_SVCT3_CORE) $(CM_SVCT3_CHECK) tests/test_cm_svct3.c -o build/test-cm-svct3-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-svct3-sanitize
c64-cm-svct3: build/cm-svct3.prg
build/cm-svct3.prg: $(CM_SVCT3_CORE) $(CM_SVCT3_CHECK) platform/c64/cm_svct3_diagnostic.c $(CM_SVCT3_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-svct3.map $(CM_SVCT3_CORE) $(CM_SVCT3_CHECK) platform/c64/cm_svct3_diagnostic.c -o $@
