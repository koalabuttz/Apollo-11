# SPDX-License-Identifier: MIT
CM_LONGCALL_CORE = src/cm_longcall.c src/agc_word.c src/agc_multiword.c tests/cm_longcall_cases.c
CM_LONGCALL_CHECK = tests/cm_longcall_check.c
CM_LONGCALL_HEADERS = include/cm_longcall.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_longcall_cases.h tests/cm_longcall_check.h tests/cm_longcall_vectors.h
.PHONY: cm-longcall-differential test-cm-longcall sanitize-cm-longcall c64-cm-longcall
all: build/cm-longcall-native build/test-cm-longcall
build/cm-longcall-native: $(CM_LONGCALL_CORE) platform/host/cm_longcall_snapshot.c $(CM_LONGCALL_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_LONGCALL_CORE) platform/host/cm_longcall_snapshot.c -o $@
cm-longcall-differential: build/cm-longcall-native
	python3 tools/cm_longcall_differential.py
build/test-cm-longcall: $(CM_LONGCALL_CORE) $(CM_LONGCALL_CHECK) tests/test_cm_longcall.c $(CM_LONGCALL_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_LONGCALL_CORE) $(CM_LONGCALL_CHECK) tests/test_cm_longcall.c -o $@
test: test-cm-longcall
test-cm-longcall: build/test-cm-longcall
	./build/test-cm-longcall
sanitize: sanitize-cm-longcall
sanitize-cm-longcall: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_LONGCALL_CORE) $(CM_LONGCALL_CHECK) tests/test_cm_longcall.c -o build/test-cm-longcall-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-longcall-sanitize
c64-cm-longcall: build/cm-longcall.prg
build/cm-longcall.prg: $(CM_LONGCALL_CORE) $(CM_LONGCALL_CHECK) platform/c64/cm_longcall_diagnostic.c $(CM_LONGCALL_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-longcall.map $(CM_LONGCALL_CORE) $(CM_LONGCALL_CHECK) platform/c64/cm_longcall_diagnostic.c -o $@
