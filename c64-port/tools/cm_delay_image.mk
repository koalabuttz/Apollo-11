# SPDX-License-Identifier: MIT
CM_DELAY_CORE = src/cm_delay_image.c src/cm_waitlist_image.c src/cm_waitlist.c src/cm_native_image.c src/agc_word.c tests/cm_delay_image_cases.c
CM_DELAY_CHECK = tests/cm_delay_image_check.c
CM_DELAY_HEADERS = include/cm_delay_image.h include/cm_phase.h include/cm_native_image.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_delay_image_cases.h tests/cm_delay_image_check.h tests/cm_delay_image_vectors.h
.PHONY: cm-delay-image-differential test-cm-delay-image sanitize-cm-delay-image c64-cm-delay-image
all: build/cm-delay-image-native build/test-cm-delay-image
build/cm-delay-image-native: $(CM_DELAY_CORE) platform/host/cm_delay_image_snapshot.c $(CM_DELAY_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DELAY_CORE) platform/host/cm_delay_image_snapshot.c -o $@
cm-delay-image-differential: build/cm-delay-image-native
	python3 tools/cm_delay_image_differential.py
build/test-cm-delay-image: $(CM_DELAY_CORE) $(CM_DELAY_CHECK) tests/test_cm_delay_image.c $(CM_DELAY_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DELAY_CORE) $(CM_DELAY_CHECK) tests/test_cm_delay_image.c -o $@
test: test-cm-delay-image
test-cm-delay-image: build/test-cm-delay-image
	./build/test-cm-delay-image
sanitize: sanitize-cm-delay-image
sanitize-cm-delay-image: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_DELAY_CORE) $(CM_DELAY_CHECK) tests/test_cm_delay_image.c -o build/test-cm-delay-image-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-delay-image-sanitize
c64-cm-delay-image: build/cm-delay-image.prg
build/cm-delay-image.prg: $(CM_DELAY_CORE) $(CM_DELAY_CHECK) platform/c64/cm_delay_image_diagnostic.c $(CM_DELAY_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-delay-image.map $(CM_DELAY_CORE) $(CM_DELAY_CHECK) platform/c64/cm_delay_image_diagnostic.c -o $@
