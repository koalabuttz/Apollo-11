# SPDX-License-Identifier: MIT
CM_ALARM2_IMAGE_CORE = src/cm_alarm2_image.c src/cm_alarm_image.c src/cm_alarm_record.c src/cm_fixed_compat.c src/cm_native_image.c src/agc_word.c tests/cm_alarm2_image_cases.c tests/cm_alarm_record_cases.c tests/cm_admission_complete_cases.c
build/cm-alarm2-image-native: $(CM_ALARM2_IMAGE_CORE) platform/host/cm_alarm2_image_snapshot.c include/cm_alarm2_image.h | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_ALARM2_IMAGE_CORE) platform/host/cm_alarm2_image_snapshot.c -o $@
cm-alarm2-image-differential: build/cm-alarm2-image-native prepare-cm-original-rope
	python3 tools/cm_alarm2_image_differential.py
.PHONY: cm-alarm2-image-differential test-cm-alarm2-image sanitize-cm-alarm2-image
test: test-cm-alarm2-image
test-cm-alarm2-image: cm-alarm2-image-differential
sanitize: sanitize-cm-alarm2-image
sanitize-cm-alarm2-image: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ALARM2_IMAGE_CORE) platform/host/cm_alarm2_image_snapshot.c -o build/cm-alarm2-image-sanitize
	python3 tools/cm_alarm2_image_sanitize.py
