# SPDX-License-Identifier: MIT
# Milestone lm-alarm (lane A2): Luminary099 ALARM_AND_ABORT (ALARM ... VARALARM,
# PRIOLARM, FLAGS, GOPOODOO) as native routines of the control-transfer ABI.
LM_AL_SRC = src/lm_alarm.c src/agc_image_ops.c src/agc_hardware_divide.c src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_alarm_cases.c
LM_AL_HDR = include/lm_alarm.h include/lm_interbank_routines.h include/agc_transfer.h include/agc_image_ops.h src/lm_alarm_sites.inc tests/lm_alarm_cases.h
.PHONY: test-lm-alarm sanitize-lm-alarm c64-lm-alarm vice-lm-alarm lm-alarm-differential
all: build/test-lm-alarm
test: test-lm-alarm
sanitize: sanitize-lm-alarm
build/test-lm-alarm: $(LM_AL_SRC) tests/test_lm_alarm.c tests/lm_alarm_vectors.h $(LM_AL_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_AL_SRC) tests/test_lm_alarm.c -o $@
test-lm-alarm: build/test-lm-alarm
	python3 tools/lm_alarm_generate.py --check
	./build/test-lm-alarm
sanitize-lm-alarm: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_AL_SRC) tests/test_lm_alarm.c -o build/test-lm-alarm-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-alarm-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-alarm-native: $(LM_AL_SRC) platform/host/lm_alarm_snapshot.c $(LM_AL_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_AL_SRC) platform/host/lm_alarm_snapshot.c -o $@
lm-alarm-differential: build/lm-alarm-native
	python3 tools/lm_alarm_freeze.py
	git diff --exit-code -- tests/lm_alarm_vectors.h tests/lm_alarm_c64_vectors.h
c64-lm-alarm: build/lm-alarm.prg
build/lm-alarm.prg: $(LM_AL_SRC) platform/c64/lm_alarm_diagnostic.c tests/lm_alarm_c64_vectors.h $(LM_AL_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-alarm.map $(LM_AL_SRC) platform/c64/lm_alarm_diagnostic.c -o $@
vice-lm-alarm: build/lm-alarm.prg
	python3 tools/vice_run.py build/lm-alarm.prg --cycles 8000000000 --timeout 2400
