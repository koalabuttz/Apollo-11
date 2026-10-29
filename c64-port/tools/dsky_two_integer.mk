# SPDX-License-Identifier: MIT
DSKY_TWO_INTEGER_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_TWO_INTEGER_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_five_blank.c src/dsky_sign_output.c src/dsky_read_pair.c src/dsky_two_integer.c tests/dsky_two_integer_cases.c tests/dsky_two_integer_check.c
all: build/test-dsky-two-integer build/dsky-two-integer-route-port
build/test-dsky-two-integer: tests/test_dsky_two_integer.c $(DSKY_TWO_INTEGER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_TWO_INTEGER_FLAGS) $^ -o $@
build/dsky-two-integer-route-port: tools/dsky_two_integer_snapshot.c $(DSKY_TWO_INTEGER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_TWO_INTEGER_FLAGS) $^ -o $@
dsky-two-integer-host: build/test-dsky-two-integer
	build/test-dsky-two-integer
dsky-two-integer-sanitize:
	python3 tools/dsky_two_integer_sanitize.py
dsky-two-integer-c64:
	python3 tools/dsky_two_integer_build.py --mos-cc $(MOS_CC)
test: dsky-two-integer-host
sanitize: dsky-two-integer-sanitize
.PHONY: dsky-two-integer-host dsky-two-integer-sanitize dsky-two-integer-c64
