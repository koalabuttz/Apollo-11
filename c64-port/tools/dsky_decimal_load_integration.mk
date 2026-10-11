# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_DECIMAL_LOAD_CORE = src/agc_word.c src/dsky_decimal_load.c tests/dsky_decimal_load_cases.c tests/dsky_decimal_load_check.c
INTEGRATED_DSKY_DECIMAL_LOAD_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-decimal-load-integrated build/dsky-decimal-load-integrated-stream
build/test-dsky-decimal-load-integrated: tests/test_dsky_decimal_load.c $(INTEGRATED_DSKY_DECIMAL_LOAD_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_LOAD_FLAGS) $^ -o $@
build/dsky-decimal-load-integrated-stream: tools/dsky_decimal_load_snapshot.c $(INTEGRATED_DSKY_DECIMAL_LOAD_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_LOAD_FLAGS) $^ -o $@
test: test-dsky-decimal-load-integrated
test-dsky-decimal-load-integrated: build/test-dsky-decimal-load-integrated build/dsky-decimal-load-integrated-stream
	./build/test-dsky-decimal-load-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-load-reference.snapshots.xz --sha256 bd726f3516b38148a8bd445206fdb75a4e6538ba95add0e71d037fcb44573fea --binary build/dsky-decimal-load-integrated-stream
sanitize: sanitize-dsky-decimal-load-integrated
sanitize-dsky-decimal-load-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_LOAD_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_decimal_load.c $(INTEGRATED_DSKY_DECIMAL_LOAD_CORE) -o build/test-dsky-decimal-load-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-decimal-load-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_LOAD_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_decimal_load_snapshot.c $(INTEGRATED_DSKY_DECIMAL_LOAD_CORE) -o build/dsky-decimal-load-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-load-reference.snapshots.xz --sha256 bd726f3516b38148a8bd445206fdb75a4e6538ba95add0e71d037fcb44573fea --binary build/dsky-decimal-load-integrated-stream-san
.PHONY: test-dsky-decimal-load-integrated sanitize-dsky-decimal-load-integrated
