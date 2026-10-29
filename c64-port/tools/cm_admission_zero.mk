# SPDX-License-Identifier: MIT
CM_ADMISSION_ZERO_CORE = src/cm_admission_zero.c src/cm_native_image.c src/agc_word.c src/cm_executive.c tests/cm_admission_zero_cases.c tests/cm_admission_complete_cases.c
build/cm-admission-zero-native: $(CM_ADMISSION_ZERO_CORE) platform/host/cm_admission_zero_snapshot.c include/cm_admission_zero.h | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_ADMISSION_ZERO_CORE) platform/host/cm_admission_zero_snapshot.c -o $@
cm-admission-zero-differential: build/cm-admission-zero-native prepare-cm-original-rope
	python3 tools/cm_admission_zero_differential.py
.PHONY: test-cm-admission-zero sanitize-cm-admission-zero
test: test-cm-admission-zero
test-cm-admission-zero: cm-admission-zero-differential
sanitize: sanitize-cm-admission-zero
sanitize-cm-admission-zero: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ADMISSION_ZERO_CORE) platform/host/cm_admission_zero_snapshot.c -o build/cm-admission-zero-sanitize
	python3 tools/cm_admission_zero_sanitize.py
