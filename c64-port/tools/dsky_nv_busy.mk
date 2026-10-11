# SPDX-License-Identifier: MIT
DSKY_NV_BUSY_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_NV_BUSY_CORE = src/agc_word.c src/dsky_nv_busy.c tests/dsky_nv_busy_cases.c tests/dsky_nv_busy_check.c
all: build/test-dsky-nv-busy build/dsky-nv-busy-route-port
build/test-dsky-nv-busy: tests/test_dsky_nv_busy.c $(DSKY_NV_BUSY_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_BUSY_FLAGS) $^ -o $@
build/dsky-nv-busy-route-port: tools/dsky_nv_busy_snapshot.c $(DSKY_NV_BUSY_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_BUSY_FLAGS) $^ -o $@
dsky-nv-busy-host: build/test-dsky-nv-busy
	build/test-dsky-nv-busy
dsky-nv-busy-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_NV_BUSY_FLAGS) tests/test_dsky_nv_busy.c $(DSKY_NV_BUSY_CORE) -o build/test-dsky-nv-busy-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-nv-busy-sanitize
	python3 tools/dsky_nv_busy_sanitize.py
dsky-nv-busy-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_NV_BUSY_FLAGS) tools/dsky_nv_busy_diagnostic.c $(DSKY_NV_BUSY_CORE) -Wl,-Map,build/dsky_nv_busy.map -o build/dsky_nv_busy.prg
test: dsky-nv-busy-host
sanitize: dsky-nv-busy-sanitize
.PHONY: dsky-nv-busy-host dsky-nv-busy-sanitize dsky-nv-busy-c64
