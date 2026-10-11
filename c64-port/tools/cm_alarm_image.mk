# SPDX-License-Identifier: MIT
CM_ALARM_IMAGE_CORE = src/cm_alarm_image.c src/cm_alarm_record.c src/cm_fixed_compat.c src/cm_native_image.c src/agc_word.c tests/cm_alarm_image_cases.c tests/cm_alarm_record_cases.c tests/cm_admission_complete_cases.c
build/cm-alarm-image-native: $(CM_ALARM_IMAGE_CORE) platform/host/cm_alarm_image_snapshot.c include/cm_alarm_image.h include/cm_alarm_record.h include/cm_fixed_compat.h src/cm_fixed_compat_words.inc | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_ALARM_IMAGE_CORE) platform/host/cm_alarm_image_snapshot.c -o $@
cm-alarm-image-differential: build/cm-alarm-image-native prepare-cm-original-rope
	python3 tools/cm_alarm_image_differential.py
	python3 tools/cm_alarm_source_audit.py
.PHONY: cm-alarm-image-differential test-cm-alarm-image sanitize-cm-alarm-image
test: test-cm-alarm-image
test-cm-alarm-image: cm-alarm-image-differential
sanitize: sanitize-cm-alarm-image
sanitize-cm-alarm-image: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ALARM_IMAGE_CORE) platform/host/cm_alarm_image_snapshot.c -o build/cm-alarm-image-sanitize
	python3 tools/cm_alarm_image_sanitize.py
