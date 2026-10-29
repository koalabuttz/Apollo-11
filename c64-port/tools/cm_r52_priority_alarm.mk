# SPDX-License-Identifier: MIT
CM_R52_PRIORITY_ALARM_CORE = src/cm_r52_priority_alarm.c src/cm_priolarm.c src/cm_priority_display.c src/cm_admission_zero.c src/cm_basic_finish.c src/cm_interbank_complete.c src/cm_executive.c src/cm_dispatch.c src/cm_native_image.c src/agc_word.c tests/cm_r52_priority_alarm_cases.c tests/cm_admission_complete_cases.c
build/cm-r52-priority-alarm-native: $(CM_R52_PRIORITY_ALARM_CORE) platform/host/cm_r52_priority_alarm_snapshot.c include/cm_r52_priority_alarm.h | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_R52_PRIORITY_ALARM_CORE) platform/host/cm_r52_priority_alarm_snapshot.c -o $@
cm-r52-priority-alarm-differential: build/cm-r52-priority-alarm-native prepare-cm-original-rope
	python3 tools/cm_r52_priority_alarm_differential.py
.PHONY: cm-r52-priority-alarm-differential test-cm-r52-priority-alarm sanitize-cm-r52-priority-alarm
test: test-cm-r52-priority-alarm
test-cm-r52-priority-alarm: cm-r52-priority-alarm-differential
sanitize: sanitize-cm-r52-priority-alarm
sanitize-cm-r52-priority-alarm: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_R52_PRIORITY_ALARM_CORE) platform/host/cm_r52_priority_alarm_snapshot.c -o build/cm-r52-priority-alarm-sanitize
	python3 tools/cm_r52_priority_alarm_sanitize.py
