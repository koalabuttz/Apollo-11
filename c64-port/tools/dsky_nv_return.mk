# SPDX-License-Identifier: MIT
DSKY_NV_RETURN_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_NV_RETURN_CORE = src/agc_word.c src/dsky_nv_return.c tests/dsky_nv_return_cases.c tests/dsky_nv_return_check.c
all: build/test-dsky-nv-return build/dsky-nv-return-route-port
build/test-dsky-nv-return: tests/test_dsky_nv_return.c $(DSKY_NV_RETURN_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_RETURN_FLAGS) $^ -o $@
build/dsky-nv-return-route-port: tools/dsky_nv_return_snapshot.c $(DSKY_NV_RETURN_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_RETURN_FLAGS) $^ -o $@
dsky-nv-return-host: build/test-dsky-nv-return
	build/test-dsky-nv-return
dsky-nv-return-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_NV_RETURN_FLAGS) tests/test_dsky_nv_return.c $(DSKY_NV_RETURN_CORE) -o build/test-dsky-nv-return-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-nv-return-sanitize
	python3 tools/dsky_nv_return_sanitize.py
dsky-nv-return-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_NV_RETURN_FLAGS) tools/dsky_nv_return_diagnostic.c $(DSKY_NV_RETURN_CORE) -Wl,-Map,build/dsky_nv_return.map -o build/dsky_nv_return.prg
test: dsky-nv-return-host
sanitize: dsky-nv-return-sanitize
.PHONY: dsky-nv-return-host dsky-nv-return-sanitize dsky-nv-return-c64
