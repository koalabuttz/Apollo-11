# SPDX-License-Identifier: MIT
DSKY_NV_BLANK_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_NV_BLANK_CORE = src/agc_word.c src/dsky_nv_blank.c tests/dsky_nv_blank_cases.c tests/dsky_nv_blank_check.c
all: build/test-dsky-nv-blank build/dsky-nv-blank-route-port
build/test-dsky-nv-blank: tests/test_dsky_nv_blank.c $(DSKY_NV_BLANK_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_BLANK_FLAGS) $^ -o $@
build/dsky-nv-blank-route-port: tools/dsky_nv_blank_snapshot.c $(DSKY_NV_BLANK_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_BLANK_FLAGS) $^ -o $@
dsky-nv-blank-host: build/test-dsky-nv-blank
	build/test-dsky-nv-blank
dsky-nv-blank-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_NV_BLANK_FLAGS) tests/test_dsky_nv_blank.c $(DSKY_NV_BLANK_CORE) -o build/test-dsky-nv-blank-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-nv-blank-sanitize
	python3 tools/dsky_nv_blank_sanitize.py
dsky-nv-blank-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_NV_BLANK_FLAGS) tools/dsky_nv_blank_diagnostic.c $(DSKY_NV_BLANK_CORE) -Wl,-Map,build/dsky_nv_blank.map -o build/dsky_nv_blank.prg
test: dsky-nv-blank-host
sanitize: dsky-nv-blank-sanitize
.PHONY: dsky-nv-blank-host dsky-nv-blank-sanitize dsky-nv-blank-c64
