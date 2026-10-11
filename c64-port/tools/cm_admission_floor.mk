# SPDX-License-Identifier: MIT
CM_ADMISSION_FLOOR_CORE = src/cm_admission_floor.c src/cm_native_image.c src/cm_executive.c src/agc_word.c tests/cm_admission_floor_cases.c tests/cm_admission_complete_cases.c tests/cm_executive_cases.c
build/cm-admission-floor-native: $(CM_ADMISSION_FLOOR_CORE) platform/host/cm_admission_floor_snapshot.c include/cm_admission_floor.h tests/cm_admission_floor_cases.h tools/cm_alarm2_image_dump.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ADMISSION_FLOOR_CORE) platform/host/cm_admission_floor_snapshot.c -o $@
build/test-cm-admission-floor: $(CM_ADMISSION_FLOOR_CORE) tests/test_cm_admission_floor.c include/cm_admission_floor.h tests/cm_admission_floor_cases.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ADMISSION_FLOOR_CORE) tests/test_cm_admission_floor.c -o $@
.PHONY: test-cm-admission-floor sanitize-cm-admission-floor cm-admission-floor-differential
test: test-cm-admission-floor
test-cm-admission-floor: cm-admission-floor-differential build/test-cm-admission-floor
	./build/test-cm-admission-floor
cm-admission-floor-differential: build/cm-admission-floor-native prepare-cm-original-rope
	python3 tools/cm_admission_floor_differential.py
sanitize: sanitize-cm-admission-floor
sanitize-cm-admission-floor: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(CM_ADMISSION_FLOOR_CORE) platform/host/cm_admission_floor_snapshot.c -o build/cm-admission-floor-sanitize
	python3 tools/cm_admission_floor_sanitize.py
