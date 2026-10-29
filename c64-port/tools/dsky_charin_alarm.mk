# SPDX-License-Identifier: MIT
DSKY_CHARIN_ALARM_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_CHARIN_ALARM_CORE = src/agc_word.c src/dsky_charin_alarm.c tests/p66_bias_cases.c tests/dsky_charin_alarm_cases.c
DSKY_CHARIN_ALARM_TEST = tests/dsky_charin_alarm_check.c tests/test_dsky_charin_alarm.c
all: build/test-dsky-charin-alarm
build/test-dsky-charin-alarm: $(DSKY_CHARIN_ALARM_CORE) $(DSKY_CHARIN_ALARM_TEST) tests/dsky_charin_alarm_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_CHARIN_ALARM_FLAGS) $(DSKY_CHARIN_ALARM_CORE) $(DSKY_CHARIN_ALARM_TEST) -o $@
test-dsky-charin-alarm: build/test-dsky-charin-alarm
	build/test-dsky-charin-alarm
sanitize-dsky-charin-alarm:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_CHARIN_ALARM_FLAGS) $(DSKY_CHARIN_ALARM_CORE) $(DSKY_CHARIN_ALARM_TEST) -o build/test-dsky-charin-alarm-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-charin-alarm-sanitize
c64-dsky-charin-alarm:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_CHARIN_ALARM_FLAGS) $(DSKY_CHARIN_ALARM_CORE) tests/dsky_charin_alarm_check.c tools/dsky_charin_alarm_diagnostic.c -Wl,-Map,build/dsky_charin_alarm.map -o build/dsky_charin_alarm.prg
differential-dsky-charin-alarm:
	python3 tools/dsky_charin_alarm_differential.py
	python3 tools/freeze_dsky_charin_alarm.py
test: test-dsky-charin-alarm
sanitize: sanitize-dsky-charin-alarm
.PHONY: all test sanitize test-dsky-charin-alarm sanitize-dsky-charin-alarm c64-dsky-charin-alarm differential-dsky-charin-alarm
