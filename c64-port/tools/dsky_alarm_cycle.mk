# SPDX-License-Identifier: MIT
DSKY_ALARM_CYCLE_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_ALARM_CYCLE_CORE = src/agc_word.c src/dsky_alarm_cycle.c tests/p66_bias_cases.c tests/dsky_alarm_cycle_cases.c
DSKY_ALARM_CYCLE_TEST = tests/dsky_alarm_cycle_check.c tests/test_dsky_alarm_cycle.c
all: build/test-dsky-alarm-cycle
build/test-dsky-alarm-cycle: $(DSKY_ALARM_CYCLE_CORE) $(DSKY_ALARM_CYCLE_TEST) tests/dsky_alarm_cycle_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_ALARM_CYCLE_FLAGS) $(DSKY_ALARM_CYCLE_CORE) $(DSKY_ALARM_CYCLE_TEST) -o $@
test-dsky-alarm-cycle: build/test-dsky-alarm-cycle
	build/test-dsky-alarm-cycle
sanitize-dsky-alarm-cycle:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_ALARM_CYCLE_FLAGS) $(DSKY_ALARM_CYCLE_CORE) $(DSKY_ALARM_CYCLE_TEST) -o build/test-dsky-alarm-cycle-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-alarm-cycle-sanitize
c64-dsky-alarm-cycle:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_ALARM_CYCLE_FLAGS) $(DSKY_ALARM_CYCLE_CORE) tests/dsky_alarm_cycle_check.c tools/dsky_alarm_cycle_diagnostic.c -Wl,-Map,build/dsky_alarm_cycle.map -o build/dsky_alarm_cycle.prg
differential-dsky-alarm-cycle:
	python3 tools/dsky_alarm_cycle_differential.py
	python3 tools/freeze_dsky_alarm_cycle.py
test: test-dsky-alarm-cycle
sanitize: sanitize-dsky-alarm-cycle
.PHONY: all test sanitize test-dsky-alarm-cycle sanitize-dsky-alarm-cycle c64-dsky-alarm-cycle differential-dsky-alarm-cycle
