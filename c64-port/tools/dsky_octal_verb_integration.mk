# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_OCTAL_VERB_CORE = src/agc_word.c src/dsky_octal_verb.c tests/dsky_octal_verb_cases.c tests/dsky_octal_verb_check.c
INTEGRATED_DSKY_OCTAL_VERB_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-octal-verb-integrated build/dsky-octal-verb-integrated-stream
build/test-dsky-octal-verb-integrated: tests/test_dsky_octal_verb.c $(INTEGRATED_DSKY_OCTAL_VERB_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_OCTAL_VERB_FLAGS) $^ -o $@
build/dsky-octal-verb-integrated-stream: tools/dsky_octal_verb_snapshot.c $(INTEGRATED_DSKY_OCTAL_VERB_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_OCTAL_VERB_FLAGS) $^ -o $@
test: test-dsky-octal-verb-integrated
test-dsky-octal-verb-integrated: build/test-dsky-octal-verb-integrated build/dsky-octal-verb-integrated-stream
	./build/test-dsky-octal-verb-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-octal-verb-reference.snapshots.xz --sha256 ad099cc1be0f840394294fe64c1a083f9ea4ae8891caf847e2f3417a650423c4 --binary build/dsky-octal-verb-integrated-stream
sanitize: sanitize-dsky-octal-verb-integrated
sanitize-dsky-octal-verb-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_OCTAL_VERB_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_octal_verb.c $(INTEGRATED_DSKY_OCTAL_VERB_CORE) -o build/test-dsky-octal-verb-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-octal-verb-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_OCTAL_VERB_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_octal_verb_snapshot.c $(INTEGRATED_DSKY_OCTAL_VERB_CORE) -o build/dsky-octal-verb-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-octal-verb-reference.snapshots.xz --sha256 ad099cc1be0f840394294fe64c1a083f9ea4ae8891caf847e2f3417a650423c4 --binary build/dsky-octal-verb-integrated-stream-san
.PHONY: test-dsky-octal-verb-integrated sanitize-dsky-octal-verb-integrated
