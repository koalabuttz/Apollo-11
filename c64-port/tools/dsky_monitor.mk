# SPDX-License-Identifier: MIT
DSKY_MONITOR_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_MONITOR_CORE = src/agc_word.c src/dsky_monitor.c tests/dsky_monitor_cases.c tests/dsky_monitor_check.c
all: build/test-dsky-monitor build/dsky-monitor-route-port
build/test-dsky-monitor: tests/test_dsky_monitor.c $(DSKY_MONITOR_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_MONITOR_FLAGS) $^ -o $@
build/dsky-monitor-route-port: tools/dsky_monitor_snapshot.c $(DSKY_MONITOR_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_MONITOR_FLAGS) $^ -o $@
test: build/test-dsky-monitor
	build/test-dsky-monitor
sanitize:
	python3 tools/dsky_monitor_sanitize.py
