# SPDX-License-Identifier: MIT
CM_ALARM_COMPLETE_CORE = src/cm_alarm_complete.c src/cm_alarm_image.c src/cm_alarm_record.c src/cm_fixed_compat.c src/cm_native_image.c src/agc_word.c tests/cm_alarm_complete_cases.c tests/cm_alarm_image_cases.c tests/cm_alarm_record_cases.c tests/cm_admission_complete_cases.c
build/cm-alarm-complete-native: $(CM_ALARM_COMPLETE_CORE) platform/host/cm_alarm_complete_snapshot.c include/cm_alarm_complete.h include/cm_alarm_record.h include/cm_fixed_compat.h src/cm_fixed_compat_words.inc | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_ALARM_COMPLETE_CORE) platform/host/cm_alarm_complete_snapshot.c -o $@
cm-alarm-complete-differential: build/cm-alarm-complete-native prepare-cm-original-rope
	python3 tools/cm_alarm_complete_differential.py
.PHONY: cm-alarm-complete-differential test-cm-alarm-complete sanitize-cm-alarm-complete
test: test-cm-alarm-complete
test-cm-alarm-complete: cm-alarm-complete-differential
sanitize: sanitize-cm-alarm-complete
sanitize-cm-alarm-complete: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ALARM_COMPLETE_CORE) platform/host/cm_alarm_complete_snapshot.c -o build/cm-alarm-complete-sanitize
	python3 tools/cm_alarm_complete_sanitize.py
