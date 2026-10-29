# SPDX-License-Identifier: MIT
NVW_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
NVW_CORE = src/agc_word.c src/p66_nv_words.c src/dsky_two_digit.c src/dsky_pinball.c src/agc_multiword.c src/p66_nv_entry.c tests/throttle_bridge_cases.c tests/nv_entry_cases.c tests/nv_words_cases.c
nv_words-host:
	gcc -O2 -Wconversion -pedantic $(NVW_FLAGS) tests/test_nv_words.c tests/nv_words_check.c $(NVW_CORE) -o build/test-nv-words
	build/test-nv-words
nv_words-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(NVW_FLAGS) tests/test_nv_words.c tests/nv_words_check.c $(NVW_CORE) -o build/test-nv-words-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-nv-words-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(NVW_FLAGS) tools/nv_words_snapshot.c $(NVW_CORE) -o build/nv-words-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/nv-words-native-sanitize > build/nv-words-sanitize.snapshots
	cmp build/nv-words-sanitize.snapshots build/nv-words-reference.snapshots
nv_words-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(NVW_FLAGS) tools/nv_words_diagnostic.c tests/nv_words_check.c $(NVW_CORE) -Wl,-Map,build/nv_words.map -o build/nv_words.prg

test: nv_words-host
sanitize: nv_words-sanitize

.PHONY: nv_words-host nv_words-sanitize nv_words-c64 nv_words-differential
all: build/nv-words-native build/test-nv-words
build/nv-words-native: tools/nv_words_snapshot.c $(NVW_CORE) | build
	$(CC) -O2 $(NVW_FLAGS) tools/nv_words_snapshot.c $(NVW_CORE) -o $@
build/test-nv-words: tests/test_nv_words.c tests/nv_words_check.c tests/nv_words_vectors.h $(NVW_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(NVW_FLAGS) tests/test_nv_words.c tests/nv_words_check.c $(NVW_CORE) -o $@
nv_words-differential:
	python3 tools/nv_words_differential.py
