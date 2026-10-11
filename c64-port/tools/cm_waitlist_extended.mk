# SPDX-License-Identifier: MIT
CM_WAIT_EXTENDED_CORE = src/cm_waitlist_extended.c src/cm_waitlist.c src/cm_native_image.c src/agc_word.c tests/cm_waitlist_extended_cases.c tests/cm_waitlist_image_cases.c
CM_WAIT_EXTENDED_CHECK = tests/cm_waitlist_extended_check.c
CM_WAIT_EXTENDED_HEADERS = include/cm_waitlist_extended.h include/cm_phase.h include/cm_native_image.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_waitlist_extended_cases.h tests/cm_waitlist_extended_check.h tests/cm_waitlist_extended_vectors.h
.PHONY: cm-waitlist-extended-differential test-cm-waitlist-extended sanitize-cm-waitlist-extended c64-cm-waitlist-extended
all: build/cm-waitlist-extended-native build/test-cm-waitlist-extended
build/cm-waitlist-extended-native: $(CM_WAIT_EXTENDED_CORE) platform/host/cm_waitlist_extended_snapshot.c $(CM_WAIT_EXTENDED_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_WAIT_EXTENDED_CORE) platform/host/cm_waitlist_extended_snapshot.c -o $@
cm-waitlist-extended-differential: build/cm-waitlist-extended-native
	python3 tools/cm_waitlist_extended_differential.py
build/test-cm-waitlist-extended: $(CM_WAIT_EXTENDED_CORE) $(CM_WAIT_EXTENDED_CHECK) tests/test_cm_waitlist_extended.c $(CM_WAIT_EXTENDED_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_WAIT_EXTENDED_CORE) $(CM_WAIT_EXTENDED_CHECK) tests/test_cm_waitlist_extended.c -o $@
test: test-cm-waitlist-extended
test-cm-waitlist-extended: build/test-cm-waitlist-extended
	./build/test-cm-waitlist-extended
sanitize: sanitize-cm-waitlist-extended
sanitize-cm-waitlist-extended: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_WAIT_EXTENDED_CORE) $(CM_WAIT_EXTENDED_CHECK) tests/test_cm_waitlist_extended.c -o build/test-cm-waitlist-extended-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-waitlist-extended-sanitize
c64-cm-waitlist-extended: build/cm-waitlist-extended.prg
build/cm-waitlist-extended.prg: $(CM_WAIT_EXTENDED_CORE) $(CM_WAIT_EXTENDED_CHECK) platform/c64/cm_waitlist_extended_diagnostic.c $(CM_WAIT_EXTENDED_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-waitlist-extended.map $(CM_WAIT_EXTENDED_CORE) $(CM_WAIT_EXTENDED_CHECK) platform/c64/cm_waitlist_extended_diagnostic.c -o $@
