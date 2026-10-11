# SPDX-License-Identifier: MIT
CM_WLIMAGE_CORE = src/cm_waitlist_image.c src/cm_waitlist.c src/cm_phase.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c tests/cm_waitlist_image_cases.c
CM_WLIMAGE_CHECK = tests/cm_waitlist_image_check.c
CM_WLIMAGE_HEADERS = include/cm_waitlist_image.h include/cm_phase.h include/cm_native_image.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_waitlist_image_cases.h tests/cm_waitlist_image_check.h tests/cm_waitlist_image_vectors.h
.PHONY: cm-waitlist-image-differential test-cm-waitlist-image sanitize-cm-waitlist-image c64-cm-waitlist-image
all: build/cm-waitlist-image-native build/test-cm-waitlist-image
build/cm-waitlist-image-native: $(CM_WLIMAGE_CORE) platform/host/cm_waitlist_image_snapshot.c $(CM_WLIMAGE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_WLIMAGE_CORE) platform/host/cm_waitlist_image_snapshot.c -o $@
cm-waitlist-image-differential: build/cm-waitlist-image-native
	python3 tools/cm_waitlist_image_differential.py
build/test-cm-waitlist-image: $(CM_WLIMAGE_CORE) $(CM_WLIMAGE_CHECK) tests/test_cm_waitlist_image.c $(CM_WLIMAGE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_WLIMAGE_CORE) $(CM_WLIMAGE_CHECK) tests/test_cm_waitlist_image.c -o $@
test: test-cm-waitlist-image
test-cm-waitlist-image: build/test-cm-waitlist-image
	./build/test-cm-waitlist-image
sanitize: sanitize-cm-waitlist-image
sanitize-cm-waitlist-image: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_WLIMAGE_CORE) $(CM_WLIMAGE_CHECK) tests/test_cm_waitlist_image.c -o build/test-cm-waitlist-image-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-waitlist-image-sanitize
c64-cm-waitlist-image: build/cm-waitlist-image.prg
build/cm-waitlist-image.prg: $(CM_WLIMAGE_CORE) $(CM_WLIMAGE_CHECK) platform/c64/cm_waitlist_image_diagnostic.c $(CM_WLIMAGE_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-waitlist-image.map $(CM_WLIMAGE_CORE) $(CM_WLIMAGE_CHECK) platform/c64/cm_waitlist_image_diagnostic.c -o $@
