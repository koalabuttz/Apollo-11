# SPDX-License-Identifier: MIT
DSKY_ALARM_RECYCLE_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_ALARM_RECYCLE_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_alarm_cycle.c src/dsky_nv_update_general.c src/fixed_data.c src/noun_reader.c src/dsky_alarm_recycle.c tests/p66_bias_cases.c tests/dsky_alarm_recycle_cases.c
DSKY_ALARM_RECYCLE_TEST = tests/dsky_alarm_recycle_check.c tests/test_dsky_alarm_recycle.c
all: build/test-dsky-alarm-recycle
build/test-dsky-alarm-recycle: $(DSKY_ALARM_RECYCLE_CORE) $(DSKY_ALARM_RECYCLE_TEST) tests/dsky_alarm_recycle_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_ALARM_RECYCLE_FLAGS) $(DSKY_ALARM_RECYCLE_CORE) $(DSKY_ALARM_RECYCLE_TEST) -o $@
test-dsky-alarm-recycle: build/test-dsky-alarm-recycle
	build/test-dsky-alarm-recycle
sanitize-dsky-alarm-recycle:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_ALARM_RECYCLE_FLAGS) $(DSKY_ALARM_RECYCLE_CORE) $(DSKY_ALARM_RECYCLE_TEST) -o build/test-dsky-alarm-recycle-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-alarm-recycle-sanitize
c64-dsky-alarm-recycle:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_ALARM_RECYCLE_FLAGS) $(DSKY_ALARM_RECYCLE_CORE) tests/dsky_alarm_recycle_check.c tools/dsky_alarm_recycle_diagnostic.c -Wl,-Map,build/dsky_alarm_recycle.map -o build/dsky_alarm_recycle.prg
differential-dsky-alarm-recycle:
	python3 tools/dsky_alarm_recycle_differential.py
	python3 tools/freeze_dsky_alarm_recycle.py
test: test-dsky-alarm-recycle
sanitize: sanitize-dsky-alarm-recycle
.PHONY: all test sanitize test-dsky-alarm-recycle sanitize-dsky-alarm-recycle c64-dsky-alarm-recycle differential-dsky-alarm-recycle
