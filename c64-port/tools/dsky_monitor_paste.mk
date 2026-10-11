# SPDX-License-Identifier: MIT
DSKY_MONITOR_PASTE_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_MONITOR_PASTE_CORE = src/agc_word.c src/dsky_monitor_paste.c src/lm_interbank.c src/cm_interbank_complete.c tests/dsky_monitor_paste_cases.c
all: build/test-dsky-monitor-paste build/dsky-monitor-paste-route-port
build/test-dsky-monitor-paste: tests/test_dsky_monitor_paste.c $(DSKY_MONITOR_PASTE_CORE) tests/dsky_monitor_paste_check.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_MONITOR_PASTE_FLAGS) $^ -o $@
build/dsky-monitor-paste-route-port: tools/dsky_monitor_paste_snapshot.c $(DSKY_MONITOR_PASTE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_MONITOR_PASTE_FLAGS) $^ -o $@
test: build/test-dsky-monitor-paste
	build/test-dsky-monitor-paste
sanitize:
	python3 tools/dsky_monitor_paste_sanitize.py
