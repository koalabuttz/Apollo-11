# SPDX-License-Identifier: MIT
DSKY_FIXED_DISPLAY_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_FIXED_DISPLAY_CORE = src/agc_word.c src/dsky_fixed_display.c src/lm_interbank.c src/cm_fixed_callers.c src/dsky_octal_display.c src/dsky_pinball.c tests/dsky_fixed_display_cases.c
all: build/test-dsky-fixed-display build/dsky-fixed-display-route-port
build/test-dsky-fixed-display: tests/test_dsky_fixed_display.c $(DSKY_FIXED_DISPLAY_CORE) tests/dsky_fixed_display_check.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_FIXED_DISPLAY_FLAGS) $^ -o $@
build/dsky-fixed-display-route-port: tools/dsky_fixed_display_snapshot.c $(DSKY_FIXED_DISPLAY_CORE) src/dsky_fixed_display_data.c src/lm_fixed_compat.c src/cm_fixed_compat.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_FIXED_DISPLAY_FLAGS) $^ -o $@
test: build/test-dsky-fixed-display
	build/test-dsky-fixed-display
sanitize:
	python3 tools/dsky_fixed_display_sanitize.py
