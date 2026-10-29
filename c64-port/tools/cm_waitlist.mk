CM_WAITLIST_CORE = src/agc_word.c src/cm_waitlist.c src/cm_waitlist_native.c
CM_WAITLIST_INPUT = tests/cm_waitlist_cases.c
CM_WAITLIST_HEADERS = include/agc_word.h include/executive.h include/native_dispatch.h include/irq_delivery.h include/cm_waitlist.h include/cm_waitlist_native.h tests/cm_waitlist_cases.h tests/cm_waitlist_check.h tests/cm_waitlist_vectors.h
all: build/cm-waitlist-snapshot build/test-cm-waitlist
build/cm-waitlist-snapshot: $(CM_WAITLIST_CORE) $(CM_WAITLIST_INPUT) platform/host/cm_waitlist_snapshot.c $(CM_WAITLIST_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_WAITLIST_CORE) $(CM_WAITLIST_INPUT) platform/host/cm_waitlist_snapshot.c -o $@
build/test-cm-waitlist: $(CM_WAITLIST_CORE) $(CM_WAITLIST_INPUT) tests/cm_waitlist_check.c tests/test_cm_waitlist.c $(CM_WAITLIST_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_WAITLIST_CORE) $(CM_WAITLIST_INPUT) tests/cm_waitlist_check.c tests/test_cm_waitlist.c -o $@
.PHONY: test-cm-waitlist sanitize-cm-waitlist cm-waitlist-differential c64-cm-waitlist
cm-waitlist-differential: build/cm-waitlist-snapshot
	python3 tools/cm_waitlist_differential.py
test: test-cm-waitlist
test-cm-waitlist: build/test-cm-waitlist
	./build/test-cm-waitlist
sanitize: sanitize-cm-waitlist
sanitize-cm-waitlist: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_WAITLIST_CORE) $(CM_WAITLIST_INPUT) tests/cm_waitlist_check.c tests/test_cm_waitlist.c -o build/test-cm-waitlist-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-waitlist-sanitize
c64-cm-waitlist: build/cm-waitlist.prg
build/cm-waitlist.prg: $(CM_WAITLIST_CORE) $(CM_WAITLIST_INPUT) tests/cm_waitlist_check.c platform/c64/cm_waitlist_diagnostic.c $(CM_WAITLIST_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-waitlist.map $(CM_WAITLIST_CORE) $(CM_WAITLIST_INPUT) tests/cm_waitlist_check.c platform/c64/cm_waitlist_diagnostic.c -o $@
