# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_FRACTION_PAIR_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_fraction_pair.c tests/dsky_fraction_pair_cases.c tests/dsky_fraction_pair_check.c
INTEGRATED_DSKY_FRACTION_PAIR_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-fraction-pair-integrated build/dsky-fraction-pair-integrated-stream
build/test-dsky-fraction-pair-integrated: tests/test_dsky_fraction_pair.c $(INTEGRATED_DSKY_FRACTION_PAIR_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_FRACTION_PAIR_FLAGS) $^ -o $@
build/dsky-fraction-pair-integrated-stream: tools/dsky_fraction_pair_snapshot.c $(INTEGRATED_DSKY_FRACTION_PAIR_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_FRACTION_PAIR_FLAGS) $^ -o $@
test: test-dsky-fraction-pair-integrated
test-dsky-fraction-pair-integrated: build/test-dsky-fraction-pair-integrated build/dsky-fraction-pair-integrated-stream
	./build/test-dsky-fraction-pair-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-fraction-pair-reference.snapshots.xz --sha256 67750f738161bf864444c2fb872d2cc0709d159a8ab45499feec31fef0dd2a13 --binary build/dsky-fraction-pair-integrated-stream
sanitize: sanitize-dsky-fraction-pair-integrated
sanitize-dsky-fraction-pair-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_FRACTION_PAIR_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_fraction_pair.c $(INTEGRATED_DSKY_FRACTION_PAIR_CORE) -o build/test-dsky-fraction-pair-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-fraction-pair-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_FRACTION_PAIR_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_fraction_pair_snapshot.c $(INTEGRATED_DSKY_FRACTION_PAIR_CORE) -o build/dsky-fraction-pair-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-fraction-pair-reference.snapshots.xz --sha256 67750f738161bf864444c2fb872d2cc0709d159a8ab45499feec31fef0dd2a13 --binary build/dsky-fraction-pair-integrated-stream-san
.PHONY: test-dsky-fraction-pair-integrated sanitize-dsky-fraction-pair-integrated
