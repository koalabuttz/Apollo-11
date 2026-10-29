# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_MONITOR_CORE = src/agc_word.c src/dsky_monitor.c tests/dsky_monitor_cases.c tests/dsky_monitor_check.c
INTEGRATED_DSKY_MONITOR_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-monitor-integrated build/dsky-monitor-integrated-stream
build/test-dsky-monitor-integrated: tests/test_dsky_monitor.c $(INTEGRATED_DSKY_MONITOR_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_MONITOR_FLAGS) $^ -o $@
build/dsky-monitor-integrated-stream: tools/dsky_monitor_snapshot.c $(INTEGRATED_DSKY_MONITOR_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_MONITOR_FLAGS) $^ -o $@
test: test-dsky-monitor-integrated
test-dsky-monitor-integrated: build/test-dsky-monitor-integrated build/dsky-monitor-integrated-stream
	./build/test-dsky-monitor-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-monitor-reference.snapshots.xz --sha256 41a17bca3a940cbecd42a984a360ef67e8522afe8445e63506abc6d9e1758c39 --binary build/dsky-monitor-integrated-stream
sanitize: sanitize-dsky-monitor-integrated
sanitize-dsky-monitor-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_MONITOR_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_monitor.c $(INTEGRATED_DSKY_MONITOR_CORE) -o build/test-dsky-monitor-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-monitor-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_MONITOR_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_monitor_snapshot.c $(INTEGRATED_DSKY_MONITOR_CORE) -o build/dsky-monitor-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-monitor-reference.snapshots.xz --sha256 41a17bca3a940cbecd42a984a360ef67e8522afe8445e63506abc6d9e1758c39 --binary build/dsky-monitor-integrated-stream-san
.PHONY: test-dsky-monitor-integrated sanitize-dsky-monitor-integrated
