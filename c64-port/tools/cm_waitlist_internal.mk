# SPDX-License-Identifier: MIT
CM_WAITLIST_INTERNAL_CORE = src/cm_waitlist_internal.c src/cm_native_image.c src/cm_waitlist.c src/agc_word.c tests/cm_waitlist_internal_cases.c tests/cm_waitlist_image_cases.c
build/cm-waitlist-internal-native: $(CM_WAITLIST_INTERNAL_CORE) platform/host/cm_waitlist_internal_snapshot.c include/cm_waitlist_internal.h tests/cm_waitlist_internal_cases.h tools/cm_alarm2_image_dump.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_WAITLIST_INTERNAL_CORE) platform/host/cm_waitlist_internal_snapshot.c -o $@
build/test-cm-waitlist-internal: $(CM_WAITLIST_INTERNAL_CORE) tests/test_cm_waitlist_internal.c include/cm_waitlist_internal.h tests/cm_waitlist_internal_cases.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_WAITLIST_INTERNAL_CORE) tests/test_cm_waitlist_internal.c -o $@
.PHONY: test-cm-waitlist-internal sanitize-cm-waitlist-internal cm-waitlist-internal-differential
test: test-cm-waitlist-internal
test-cm-waitlist-internal: cm-waitlist-internal-differential build/test-cm-waitlist-internal
	./build/test-cm-waitlist-internal
cm-waitlist-internal-differential: build/cm-waitlist-internal-native prepare-cm-original-rope
	python3 tools/cm_waitlist_internal_differential.py
sanitize: sanitize-cm-waitlist-internal
sanitize-cm-waitlist-internal: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(CM_WAITLIST_INTERNAL_CORE) platform/host/cm_waitlist_internal_snapshot.c -o build/cm-waitlist-internal-sanitize
	python3 tools/cm_waitlist_internal_sanitize.py
