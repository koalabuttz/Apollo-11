# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_DECIMAL_GENERAL_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_decimal_general.c tests/dsky_decimal_general_cases.c tests/dsky_decimal_general_check.c
INTEGRATED_DSKY_DECIMAL_GENERAL_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-decimal-general-integrated build/dsky-decimal-general-integrated-stream
build/test-dsky-decimal-general-integrated: tests/test_dsky_decimal_general.c $(INTEGRATED_DSKY_DECIMAL_GENERAL_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_GENERAL_FLAGS) $^ -o $@
build/dsky-decimal-general-integrated-stream: tools/dsky_decimal_general_snapshot.c $(INTEGRATED_DSKY_DECIMAL_GENERAL_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_GENERAL_FLAGS) $^ -o $@
test: test-dsky-decimal-general-integrated
test-dsky-decimal-general-integrated: build/test-dsky-decimal-general-integrated build/dsky-decimal-general-integrated-stream
	./build/test-dsky-decimal-general-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-general-reference.snapshots.xz --sha256 9ba33183af2425b34d3dd1010c9f6a1729659e1c32c29c6c35f2fd13b85ede83 --binary build/dsky-decimal-general-integrated-stream
sanitize: sanitize-dsky-decimal-general-integrated
sanitize-dsky-decimal-general-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_GENERAL_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_decimal_general.c $(INTEGRATED_DSKY_DECIMAL_GENERAL_CORE) -o build/test-dsky-decimal-general-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-decimal-general-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_GENERAL_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_decimal_general_snapshot.c $(INTEGRATED_DSKY_DECIMAL_GENERAL_CORE) -o build/dsky-decimal-general-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-general-reference.snapshots.xz --sha256 9ba33183af2425b34d3dd1010c9f6a1729659e1c32c29c6c35f2fd13b85ede83 --binary build/dsky-decimal-general-integrated-stream-san
.PHONY: test-dsky-decimal-general-integrated sanitize-dsky-decimal-general-integrated
