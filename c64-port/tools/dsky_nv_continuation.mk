# SPDX-License-Identifier: MIT
DSKY_NV_CONTINUATION_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_NV_CONTINUATION_CORE = src/agc_word.c src/dsky_nv_continuation.c src/dsky_nv_return.c tests/dsky_nv_continuation_cases.c tests/dsky_nv_continuation_check.c
all: build/test-dsky-nv-continuation build/dsky-nv-continuation-route-port
build/test-dsky-nv-continuation: tests/test_dsky_nv_continuation.c $(DSKY_NV_CONTINUATION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_CONTINUATION_FLAGS) $^ -o $@
build/dsky-nv-continuation-route-port: tools/dsky_nv_continuation_snapshot.c $(DSKY_NV_CONTINUATION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_CONTINUATION_FLAGS) $^ -o $@
dsky-nv-continuation-host: build/test-dsky-nv-continuation
	build/test-dsky-nv-continuation
dsky-nv-continuation-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_NV_CONTINUATION_FLAGS) tests/test_dsky_nv_continuation.c $(DSKY_NV_CONTINUATION_CORE) -o build/test-dsky-nv-continuation-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-nv-continuation-sanitize
	python3 tools/dsky_nv_continuation_sanitize.py
dsky-nv-continuation-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_NV_CONTINUATION_FLAGS) tools/dsky_nv_continuation_diagnostic.c $(DSKY_NV_CONTINUATION_CORE) -Wl,-Map,build/dsky_nv_continuation.map -o build/dsky_nv_continuation.prg
test: dsky-nv-continuation-host
sanitize: dsky-nv-continuation-sanitize
.PHONY: dsky-nv-continuation-host dsky-nv-continuation-sanitize dsky-nv-continuation-c64
