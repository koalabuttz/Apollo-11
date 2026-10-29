# SPDX-License-Identifier: MIT
CM_MS_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_MS_CORE = src/cm_minutes_seconds.c src/cm_decimal_pair.c src/cm_pinball.c src/dsky_pinball.c src/cm_interbank.c src/fixed_data.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/cm_minutes_seconds_cases.c tests/cm_decimal_pair_cases.c tests/cm_minutes_seconds_check.c
cm_minutes_seconds-host:
	$(CC) -O2 $(CM_MS_FLAGS) tests/test_cm_minutes_seconds.c $(CM_MS_CORE) -o build/test-cm-minutes-seconds
	build/test-cm-minutes-seconds
cm_minutes_seconds-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(CM_MS_FLAGS) tests/test_cm_minutes_seconds.c $(CM_MS_CORE) -o build/test-cm-minutes-seconds-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-minutes-seconds-sanitize
	python3 tools/cm_decimal_pair_sanitize_stream.py
	python3 tools/cm_minutes_seconds_sanitize_stream.py
cm_minutes_seconds-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_MS_FLAGS) tools/cm_minutes_seconds_diagnostic.c $(CM_MS_CORE) -Wl,-Map,build/cm_minutes_seconds.map -o build/cm_minutes_seconds.prg
cm_minutes_seconds-differential:
	python3 tools/cm_decimal_pair_differential.py
	python3 tools/cm_minutes_seconds_differential.py
.PHONY: cm_minutes_seconds-host cm_minutes_seconds-sanitize cm_minutes_seconds-c64 cm_minutes_seconds-differential
