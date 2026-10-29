# SPDX-License-Identifier: MIT
DSKY_SOURCE_ALARM_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_SOURCE_ALARM_CORE = src/agc_word.c src/dsky_source_alarm.c tests/p66_bias_cases.c tests/dsky_source_alarm_cases.c
DSKY_SOURCE_ALARM_TEST = tests/dsky_source_alarm_check.c tests/test_dsky_source_alarm.c
all: build/test-dsky-source-alarm
build/test-dsky-source-alarm: $(DSKY_SOURCE_ALARM_CORE) $(DSKY_SOURCE_ALARM_TEST) tests/dsky_source_alarm_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_SOURCE_ALARM_FLAGS) $(DSKY_SOURCE_ALARM_CORE) $(DSKY_SOURCE_ALARM_TEST) -o $@
test-dsky-source-alarm: build/test-dsky-source-alarm
	build/test-dsky-source-alarm
sanitize-dsky-source-alarm:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_SOURCE_ALARM_FLAGS) $(DSKY_SOURCE_ALARM_CORE) $(DSKY_SOURCE_ALARM_TEST) -o build/test-dsky-source-alarm-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-source-alarm-sanitize
c64-dsky-source-alarm:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_SOURCE_ALARM_FLAGS) $(DSKY_SOURCE_ALARM_CORE) tests/dsky_source_alarm_check.c tools/dsky_source_alarm_diagnostic.c -Wl,-Map,build/dsky_source_alarm.map -o build/dsky_source_alarm.prg
differential-dsky-source-alarm:
	python3 tools/dsky_source_alarm_differential.py
	python3 tools/freeze_dsky_source_alarm.py
test: test-dsky-source-alarm
sanitize: sanitize-dsky-source-alarm
.PHONY: all test sanitize test-dsky-source-alarm sanitize-dsky-source-alarm c64-dsky-source-alarm differential-dsky-source-alarm
