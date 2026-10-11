# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_MONITOR_PASTE_CORE = src/agc_word.c src/dsky_monitor_paste.c src/lm_interbank.c src/cm_interbank_complete.c tests/dsky_monitor_paste_cases.c
INTEGRATED_DSKY_MONITOR_PASTE_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-monitor-paste-integrated build/dsky-monitor-paste-integrated-stream
build/test-dsky-monitor-paste-integrated: tests/test_dsky_monitor_paste.c $(INTEGRATED_DSKY_MONITOR_PASTE_CORE) tests/dsky_monitor_paste_check.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_MONITOR_PASTE_FLAGS) $^ -o $@
build/dsky-monitor-paste-integrated-stream: tools/dsky_monitor_paste_snapshot.c $(INTEGRATED_DSKY_MONITOR_PASTE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_MONITOR_PASTE_FLAGS) $^ -o $@
test: test-dsky-monitor-paste-integrated
test-dsky-monitor-paste-integrated: build/test-dsky-monitor-paste-integrated build/dsky-monitor-paste-integrated-stream
	./build/test-dsky-monitor-paste-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-monitor-paste-reference.snapshots.xz --sha256 3a9ed867a6105c586afe39a351f7471130991b4e81c266463aa66531c3de068a --binary build/dsky-monitor-paste-integrated-stream
sanitize: sanitize-dsky-monitor-paste-integrated
sanitize-dsky-monitor-paste-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_MONITOR_PASTE_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_monitor_paste.c $(INTEGRATED_DSKY_MONITOR_PASTE_CORE) tests/dsky_monitor_paste_check.c -o build/test-dsky-monitor-paste-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-monitor-paste-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_MONITOR_PASTE_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_monitor_paste_snapshot.c $(INTEGRATED_DSKY_MONITOR_PASTE_CORE) -o build/dsky-monitor-paste-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-monitor-paste-reference.snapshots.xz --sha256 3a9ed867a6105c586afe39a351f7471130991b4e81c266463aa66531c3de068a --binary build/dsky-monitor-paste-integrated-stream-san
.PHONY: test-dsky-monitor-paste-integrated sanitize-dsky-monitor-paste-integrated
