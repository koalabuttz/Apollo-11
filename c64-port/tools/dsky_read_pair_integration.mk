# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_READ_PAIR_CORE = src/agc_word.c src/dsky_read_pair.c tests/dsky_read_pair_cases.c tests/dsky_read_pair_check.c
INTEGRATED_DSKY_READ_PAIR_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-read-pair-integrated build/dsky-read-pair-integrated-stream
build/test-dsky-read-pair-integrated: tests/test_dsky_read_pair.c $(INTEGRATED_DSKY_READ_PAIR_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_READ_PAIR_FLAGS) $^ -o $@
build/dsky-read-pair-integrated-stream: tools/dsky_read_pair_snapshot.c $(INTEGRATED_DSKY_READ_PAIR_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_READ_PAIR_FLAGS) $^ -o $@
test: test-dsky-read-pair-integrated
test-dsky-read-pair-integrated: build/test-dsky-read-pair-integrated build/dsky-read-pair-integrated-stream
	./build/test-dsky-read-pair-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-read-pair-reference.snapshots.xz --sha256 e986d47143a7f3ea681bc3f1533ed885661fbbffd401604ca1661600e54d2155 --binary build/dsky-read-pair-integrated-stream
sanitize: sanitize-dsky-read-pair-integrated
sanitize-dsky-read-pair-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_READ_PAIR_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_read_pair.c $(INTEGRATED_DSKY_READ_PAIR_CORE) -o build/test-dsky-read-pair-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-read-pair-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_READ_PAIR_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_read_pair_snapshot.c $(INTEGRATED_DSKY_READ_PAIR_CORE) -o build/dsky-read-pair-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-read-pair-reference.snapshots.xz --sha256 e986d47143a7f3ea681bc3f1533ed885661fbbffd401604ca1661600e54d2155 --binary build/dsky-read-pair-integrated-stream-san
.PHONY: test-dsky-read-pair-integrated sanitize-dsky-read-pair-integrated
