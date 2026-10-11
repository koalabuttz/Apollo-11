# SPDX-License-Identifier: MIT
CM_NV85_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_NV85_CORE = src/cm_nv85.c src/cm_formatter.c src/cm_pinball.c src/dsky_two_digit.c src/dsky_decimal.c src/dsky_pinball.c src/agc_word.c src/agc_multiword.c tests/cm_nv85_cases.c tests/cm_clockjob_cases.c tests/cm_trig_cases.c tests/cm_nv85_check.c
cm_nv85-host:
	$(CC) -O2 $(CM_NV85_FLAGS) tests/test_cm_nv85.c $(CM_NV85_CORE) -o build/test-cm-nv85
	build/test-cm-nv85
cm_nv85-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(CM_NV85_FLAGS) tests/test_cm_nv85.c $(CM_NV85_CORE) -o build/test-cm-nv85-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-nv85-sanitize
	python3 tools/cm_nv85_sanitize_stream.py
cm_nv85-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_NV85_FLAGS) tools/cm_nv85_diagnostic.c $(CM_NV85_CORE) -Wl,-Map,build/cm_nv85.map -o build/cm_nv85.prg
cm_nv85-differential:
	python3 tools/cm_nv85_differential.py
.PHONY: cm_nv85-host cm_nv85-sanitize cm_nv85-c64 cm_nv85-differential
