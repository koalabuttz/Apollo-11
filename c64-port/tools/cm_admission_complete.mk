# SPDX-License-Identifier: MIT
CM_ADMISSION_COMPLETE_CORE = src/cm_admission_complete.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_admission_complete_cases.c
build/cm-admission-complete-native: $(CM_ADMISSION_COMPLETE_CORE) platform/host/cm_admission_complete_snapshot.c include/cm_admission_complete.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ADMISSION_COMPLETE_CORE) platform/host/cm_admission_complete_snapshot.c -o $@
cm-admission-complete-differential: build/cm-admission-complete-native prepare-cm-original-rope
	python3 tools/cm_admission_complete_differential.py
.PHONY: test-cm-admission-complete sanitize-cm-admission-complete
test: test-cm-admission-complete
test-cm-admission-complete: cm-admission-complete-differential
sanitize: sanitize-cm-admission-complete
sanitize-cm-admission-complete: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ADMISSION_COMPLETE_CORE) platform/host/cm_admission_complete_snapshot.c -o build/cm-admission-complete-sanitize
	python3 tools/cm_admission_complete_sanitize.py
