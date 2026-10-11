# SPDX-License-Identifier: MIT
CM_ADMISSION_CORE = src/cm_admission_image.c src/cm_executive.c src/cm_phase.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c tests/cm_admission_image_cases.c
CM_ADMISSION_CHECK = tests/cm_admission_image_check.c
CM_ADMISSION_HEADERS = include/cm_admission_image.h include/cm_phase.h include/cm_native_image.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_admission_image_cases.h tests/cm_admission_image_check.h tests/cm_admission_image_vectors.h
.PHONY: cm-admission-image-differential test-cm-admission-image sanitize-cm-admission-image c64-cm-admission-image
all: build/cm-admission-image-native build/test-cm-admission-image
build/cm-admission-image-native: $(CM_ADMISSION_CORE) platform/host/cm_admission_image_snapshot.c $(CM_ADMISSION_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ADMISSION_CORE) platform/host/cm_admission_image_snapshot.c -o $@
cm-admission-image-differential: build/cm-admission-image-native
	python3 tools/cm_admission_image_differential.py
build/test-cm-admission-image: $(CM_ADMISSION_CORE) $(CM_ADMISSION_CHECK) tests/test_cm_admission_image.c $(CM_ADMISSION_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ADMISSION_CORE) $(CM_ADMISSION_CHECK) tests/test_cm_admission_image.c -o $@
test: test-cm-admission-image
test-cm-admission-image: build/test-cm-admission-image
	./build/test-cm-admission-image
sanitize: sanitize-cm-admission-image
sanitize-cm-admission-image: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ADMISSION_CORE) $(CM_ADMISSION_CHECK) tests/test_cm_admission_image.c -o build/test-cm-admission-image-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-admission-image-sanitize
c64-cm-admission-image: build/cm-admission-image.prg
build/cm-admission-image.prg: $(CM_ADMISSION_CORE) $(CM_ADMISSION_CHECK) platform/c64/cm_admission_image_diagnostic.c $(CM_ADMISSION_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-admission-image.map $(CM_ADMISSION_CORE) $(CM_ADMISSION_CHECK) platform/c64/cm_admission_image_diagnostic.c -o $@
