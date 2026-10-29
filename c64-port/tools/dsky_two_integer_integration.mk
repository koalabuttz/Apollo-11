# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_TWO_INTEGER_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_five_blank.c src/dsky_sign_output.c src/dsky_read_pair.c src/dsky_two_integer.c tests/dsky_two_integer_cases.c tests/dsky_two_integer_check.c
INTEGRATED_DSKY_TWO_INTEGER_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-two-integer-integrated build/dsky-two-integer-integrated-stream
build/test-dsky-two-integer-integrated: tests/test_dsky_two_integer.c $(INTEGRATED_DSKY_TWO_INTEGER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_TWO_INTEGER_FLAGS) $^ -o $@
build/dsky-two-integer-integrated-stream: tools/dsky_two_integer_snapshot.c $(INTEGRATED_DSKY_TWO_INTEGER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_TWO_INTEGER_FLAGS) $^ -o $@
test: test-dsky-two-integer-integrated
test-dsky-two-integer-integrated: build/test-dsky-two-integer-integrated build/dsky-two-integer-integrated-stream
	./build/test-dsky-two-integer-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-two-integer-reference.snapshots.xz --sha256 54c0b921a14bbbea2e3a1c79ba958a4087efb934032815afcb6513d6c58d9b67 --binary build/dsky-two-integer-integrated-stream
sanitize: sanitize-dsky-two-integer-integrated
sanitize-dsky-two-integer-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_TWO_INTEGER_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_two_integer.c $(INTEGRATED_DSKY_TWO_INTEGER_CORE) -o build/test-dsky-two-integer-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-two-integer-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_TWO_INTEGER_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_two_integer_snapshot.c $(INTEGRATED_DSKY_TWO_INTEGER_CORE) -o build/dsky-two-integer-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-two-integer-reference.snapshots.xz --sha256 54c0b921a14bbbea2e3a1c79ba958a4087efb934032815afcb6513d6c58d9b67 --binary build/dsky-two-integer-integrated-stream-san
.PHONY: test-dsky-two-integer-integrated sanitize-dsky-two-integer-integrated
