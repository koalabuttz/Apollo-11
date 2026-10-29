# SPDX-License-Identifier: MIT
DSKY_NV_BUSY_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_NV_BUSY_CORE = src/agc_word.c src/dsky_nv_busy.c tests/dsky_nv_busy_cases.c tests/dsky_nv_busy_check.c
all: build/test-dsky-nv-busy build/dsky-nv-busy-route-port
build/test-dsky-nv-busy: tests/test_dsky_nv_busy.c $(DSKY_NV_BUSY_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_BUSY_FLAGS) $^ -o $@
build/dsky-nv-busy-route-port: tools/dsky_nv_busy_snapshot.c $(DSKY_NV_BUSY_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_BUSY_FLAGS) $^ -o $@

test: test-dsky-nv-busy-integrated
test-dsky-nv-busy-integrated: build/test-dsky-nv-busy build/dsky-nv-busy-route-port
	./build/test-dsky-nv-busy
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-busy-reference.snapshots.xz --sha256 bb82322090522e0a3930ee9d2d94696c167fbfaa50964e06fd8596e8be8d7bd8 --binary build/dsky-nv-busy-route-port
sanitize: sanitize-dsky-nv-busy-integrated
sanitize-dsky-nv-busy-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_nv_busy.c src/agc_word.c src/dsky_nv_busy.c tests/dsky_nv_busy_cases.c tests/dsky_nv_busy_check.c -o build/test-dsky-nv-busy-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-nv-busy-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_nv_busy_snapshot.c src/agc_word.c src/dsky_nv_busy.c tests/dsky_nv_busy_cases.c tests/dsky_nv_busy_check.c -o build/dsky-nv-busy-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-busy-reference.snapshots.xz --sha256 bb82322090522e0a3930ee9d2d94696c167fbfaa50964e06fd8596e8be8d7bd8 --binary build/dsky-nv-busy-integrated-stream-san
.PHONY: test-dsky-nv-busy-integrated sanitize-dsky-nv-busy-integrated
