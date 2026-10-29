# SPDX-License-Identifier: MIT
LM_RCS_MONITOR_INTEGRATION_CORE = src/lm_rcs_monitor.c src/executive.c src/agc_word.c tests/lm_rcs_monitor_cases.c
all: build/test-lm-rcs-monitor-integrated build/lm-rcs-monitor-integrated-stream
build/test-lm-rcs-monitor-integrated: tests/test_lm_rcs_monitor.c tests/frozen_words.c $(LM_RCS_MONITOR_INTEGRATION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
build/lm-rcs-monitor-integrated-stream: tools/lm_rcs_monitor_snapshot.c $(LM_RCS_MONITOR_INTEGRATION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
test: test-lm-rcs-monitor-integrated
test-lm-rcs-monitor-integrated: build/test-lm-rcs-monitor-integrated build/lm-rcs-monitor-integrated-stream
	./build/test-lm-rcs-monitor-integrated
	python3 tools/qualified_stream_host.py --archive evidence/lm-rcs-monitor-complete.snapshots.xz --sha256 7081319c4e0305be2f3f4589c16399ff5775b454e3f232e2ac06fcbe35da60f5 --bytes 538181632 --binary build/lm-rcs-monitor-integrated-stream
sanitize: sanitize-lm-rcs-monitor-integrated
sanitize-lm-rcs-monitor-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_lm_rcs_monitor.c tests/frozen_words.c $(LM_RCS_MONITOR_INTEGRATION_CORE) -o build/test-lm-rcs-monitor-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-rcs-monitor-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/lm_rcs_monitor_snapshot.c $(LM_RCS_MONITOR_INTEGRATION_CORE) -o build/lm-rcs-monitor-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/lm-rcs-monitor-complete.snapshots.xz --sha256 7081319c4e0305be2f3f4589c16399ff5775b454e3f232e2ac06fcbe35da60f5 --bytes 538181632 --binary build/lm-rcs-monitor-integrated-stream-san
.PHONY: test-lm-rcs-monitor-integrated sanitize-lm-rcs-monitor-integrated
