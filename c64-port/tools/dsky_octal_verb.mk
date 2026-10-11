# SPDX-License-Identifier: MIT
DSKY_OCTAL_VERB_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_OCTAL_VERB_CORE = src/agc_word.c src/dsky_octal_verb.c tests/dsky_octal_verb_cases.c tests/dsky_octal_verb_check.c
all: build/test-dsky-octal-verb build/dsky-octal-verb-route-port
build/test-dsky-octal-verb: tests/test_dsky_octal_verb.c $(DSKY_OCTAL_VERB_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_OCTAL_VERB_FLAGS) $^ -o $@
build/dsky-octal-verb-route-port: tools/dsky_octal_verb_snapshot.c $(DSKY_OCTAL_VERB_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_OCTAL_VERB_FLAGS) $^ -o $@
dsky-octal-verb-host: build/test-dsky-octal-verb
	build/test-dsky-octal-verb
dsky-octal-verb-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_OCTAL_VERB_FLAGS) tests/test_dsky_octal_verb.c $(DSKY_OCTAL_VERB_CORE) -o build/test-dsky-octal-verb-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-octal-verb-sanitize
	python3 tools/dsky_octal_verb_sanitize.py
dsky-octal-verb-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_OCTAL_VERB_FLAGS) tools/dsky_octal_verb_diagnostic.c $(DSKY_OCTAL_VERB_CORE) -Wl,-Map,build/dsky_octal_verb.map -o build/dsky_octal_verb.prg
test: dsky-octal-verb-host
sanitize: dsky-octal-verb-sanitize
.PHONY: dsky-octal-verb-host dsky-octal-verb-sanitize dsky-octal-verb-c64
