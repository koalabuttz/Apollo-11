# SPDX-License-Identifier: MIT
CM_ALARM_RECORD_CORE = src/cm_alarm_record.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_alarm_record_cases.c tests/cm_admission_complete_cases.c
build/cm-alarm-record-native: $(CM_ALARM_RECORD_CORE) platform/host/cm_alarm_record_snapshot.c include/cm_alarm_record.h | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_ALARM_RECORD_CORE) platform/host/cm_alarm_record_snapshot.c -o $@
cm-alarm-record-differential: build/cm-alarm-record-native prepare-cm-original-rope
	python3 tools/cm_alarm_record_differential.py
.PHONY: test-cm-alarm-record sanitize-cm-alarm-record
test: test-cm-alarm-record
test-cm-alarm-record: cm-alarm-record-differential
sanitize: sanitize-cm-alarm-record
sanitize-cm-alarm-record: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ALARM_RECORD_CORE) platform/host/cm_alarm_record_snapshot.c -o build/cm-alarm-record-sanitize
	python3 tools/cm_alarm_record_sanitize.py
