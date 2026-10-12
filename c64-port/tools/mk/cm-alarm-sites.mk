# SPDX-License-Identifier: MIT
# Milestone cm-alarm-sites (lane A): Comanche055 ALARM_AND_ABORT inline operands
# from compiled call-site tables instead of the original rope.
CM_ALARM_SITES_SRC = src/cm_alarm_sites.c src/agc_transfer.c src/cm_alarm_complete.c src/cm_alarm_image.c src/cm_alarm2_image.c src/cm_abort_word.c src/cm_alarm_record.c src/cm_native_image.c src/agc_word.c
CM_ALARM_SITES_CASES = tests/cm_alarm_sites_cases.c tests/cm_alarm_complete_cases.c tests/cm_alarm_image_cases.c tests/cm_alarm2_image_cases.c tests/cm_abort_word_cases.c tests/cm_alarm_record_cases.c tests/cm_admission_complete_cases.c
CM_ALARM_SITES_HDR = include/agc_transfer.h include/cm_alarm_sites.h src/cm_alarm_sites.inc tests/cm_alarm_sites_cases.h
.PHONY: test-cm-alarm-sites sanitize-cm-alarm-sites c64-cm-alarm-sites vice-cm-alarm-sites
all: build/cm-alarm-sites-native build/test-cm-alarm-sites
test: test-cm-alarm-sites
sanitize: sanitize-cm-alarm-sites
build/cm-alarm-sites-native: $(CM_ALARM_SITES_SRC) $(CM_ALARM_SITES_CASES) platform/host/cm_alarm_sites_snapshot.c $(CM_ALARM_SITES_HDR) | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_ALARM_SITES_SRC) $(CM_ALARM_SITES_CASES) platform/host/cm_alarm_sites_snapshot.c -o $@
build/test-cm-alarm-sites: $(CM_ALARM_SITES_SRC) tests/test_cm_alarm_sites.c $(CM_ALARM_SITES_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ALARM_SITES_SRC) tests/test_cm_alarm_sites.c -o $@
test-cm-alarm-sites: build/test-cm-alarm-sites build/cm-alarm-sites-native
	./build/test-cm-alarm-sites
	python3 tools/cm_alarm_sites_differential.py
sanitize-cm-alarm-sites: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ALARM_SITES_SRC) tests/test_cm_alarm_sites.c -o build/test-cm-alarm-sites-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-alarm-sites-sanitize
	$(CC) $(CPPFLAGS) -Itools -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ALARM_SITES_SRC) $(CM_ALARM_SITES_CASES) platform/host/cm_alarm_sites_snapshot.c -o build/cm-alarm-sites-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/cm-alarm-sites-sanitize | sha256sum
c64-cm-alarm-sites: build/cm-alarm-sites.prg
build/cm-alarm-sites.prg: $(CM_ALARM_SITES_SRC) $(CM_ALARM_SITES_CASES) platform/c64/cm_alarm_sites_diagnostic.c $(CM_ALARM_SITES_HDR) build/cm-alarm-sites-vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -Ibuild -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-alarm-sites.map $(CM_ALARM_SITES_SRC) $(CM_ALARM_SITES_CASES) platform/c64/cm_alarm_sites_diagnostic.c -o $@
build/cm-alarm-sites-vectors.h: build/cm-alarm-sites-native tools/cm_alarm_sites_vectors.py
	python3 tools/cm_alarm_sites_vectors.py
vice-cm-alarm-sites: build/cm-alarm-sites.prg
	python3 tools/vice_run.py build/cm-alarm-sites.prg --cycles 3000000000 --timeout 1200
