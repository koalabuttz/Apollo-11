# SPDX-License-Identifier: MIT
DSKY_OPERATOR_SCHEDULE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_OPERATOR_SCHEDULE_CORE = src/agc_word.c src/executive.c src/p66_native.c src/lm_admission_image.c src/waitlist.c src/lm_waitlist_image.c src/lm_basic_wake.c src/agc_multiword.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/cm_basic_wake.c src/cm_admission_complete.c src/cm_waitlist_extended.c src/dsky_display_release.c src/dsky_request_control.c src/dsky_operator_request.c src/dsky_operator_schedule.c tests/p66_bias_cases.c tests/dsky_operator_schedule_cases.c
DSKY_OPERATOR_SCHEDULE_TEST = tests/dsky_operator_schedule_check.c tests/test_dsky_operator_schedule.c
all: build/test-dsky-operator-schedule
build/test-dsky-operator-schedule: $(DSKY_OPERATOR_SCHEDULE_CORE) $(DSKY_OPERATOR_SCHEDULE_TEST) tests/dsky_operator_schedule_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_OPERATOR_SCHEDULE_FLAGS) $(DSKY_OPERATOR_SCHEDULE_CORE) $(DSKY_OPERATOR_SCHEDULE_TEST) -o $@
test-dsky-operator-schedule: build/test-dsky-operator-schedule
	build/test-dsky-operator-schedule
sanitize-dsky-operator-schedule:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_OPERATOR_SCHEDULE_FLAGS) $(DSKY_OPERATOR_SCHEDULE_CORE) $(DSKY_OPERATOR_SCHEDULE_TEST) -o build/test-dsky-operator-schedule-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-operator-schedule-sanitize
c64-dsky-operator-schedule:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_OPERATOR_SCHEDULE_TARGET_LM $(DSKY_OPERATOR_SCHEDULE_FLAGS) $(DSKY_OPERATOR_SCHEDULE_CORE) tests/dsky_operator_schedule_check.c tools/dsky_operator_schedule_diagnostic.c -Wl,-Map,build/dsky_operator_schedule-lm.map -o build/dsky_operator_schedule-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_OPERATOR_SCHEDULE_TARGET_CM $(DSKY_OPERATOR_SCHEDULE_FLAGS) $(DSKY_OPERATOR_SCHEDULE_CORE) tests/dsky_operator_schedule_check.c tools/dsky_operator_schedule_diagnostic.c -Wl,-Map,build/dsky_operator_schedule-cm.map -o build/dsky_operator_schedule-cm.prg
differential-dsky-operator-schedule:
	python3 tools/dsky_operator_schedule_differential.py
	python3 tools/freeze_dsky_operator_schedule.py
test: test-dsky-operator-schedule
sanitize: sanitize-dsky-operator-schedule
.PHONY: all test sanitize test-dsky-operator-schedule sanitize-dsky-operator-schedule c64-dsky-operator-schedule differential-dsky-operator-schedule
