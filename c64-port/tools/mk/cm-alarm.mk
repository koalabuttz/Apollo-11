# SPDX-License-Identifier: MIT
# Milestone cm-alarm (lane A): Comanche055 ALARM_AND_ABORT (ALARM, ALARM2, BAILOUT, POODOO, CCSHOLE, CURTAINS, VARALARM,
# PRIOLARM) as native routines of the control-transfer ABI.
CM_AL_SRC = src/cm_alarm.c src/agc_image_ops.c src/agc_hardware_divide.c src/cm_interbank_routines.c src/cm_interbank_idle.c src/cm_interbank_complete.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_alarm_cases.c
CM_AL_HDR = include/cm_alarm.h include/cm_interbank_routines.h include/agc_transfer.h include/agc_image_ops.h src/cm_alarm_operands.inc tests/cm_alarm_cases.h
.PHONY: test-cm-alarm sanitize-cm-alarm c64-cm-alarm vice-cm-alarm cm-alarm-differential
all: build/test-cm-alarm
test: test-cm-alarm
sanitize: sanitize-cm-alarm
build/test-cm-alarm: $(CM_AL_SRC) tests/test_cm_alarm.c tests/cm_alarm_vectors.h $(CM_AL_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_AL_SRC) tests/test_cm_alarm.c -o $@
test-cm-alarm: build/test-cm-alarm
	python3 tools/cm_alarm_generate.py --check
	./build/test-cm-alarm
sanitize-cm-alarm: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_AL_SRC) tests/test_cm_alarm.c -o build/test-cm-alarm-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-alarm-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-alarm-native: $(CM_AL_SRC) platform/host/cm_alarm_snapshot.c $(CM_AL_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_AL_SRC) platform/host/cm_alarm_snapshot.c -o $@
cm-alarm-differential: build/cm-alarm-native
	python3 tools/cm_alarm_freeze.py
	git diff --exit-code -- tests/cm_alarm_vectors.h tests/cm_alarm_c64_vectors.h
c64-cm-alarm: build/cm-alarm.prg
build/cm-alarm.prg: $(CM_AL_SRC) platform/c64/cm_alarm_diagnostic.c tests/cm_alarm_c64_vectors.h tests/cm_alarm_c64_rows.h $(CM_AL_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-alarm.map $(CM_AL_SRC) platform/c64/cm_alarm_diagnostic.c -o $@
vice-cm-alarm: build/cm-alarm.prg
	python3 tools/vice_run.py build/cm-alarm.prg --cycles 8000000000 --timeout 2400
