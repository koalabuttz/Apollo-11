WAIT_ZERO_CORE = src/agc_word.c src/waitlist.c
WAIT_ZERO_TEST = tests/waitlist_zero_cases.c tests/waitlist_zero_check.c
.PHONY: test-waitlist-zero sanitize-waitlist-zero c64-waitlist-zero differential-waitlist-zero
test-waitlist-zero:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WAIT_ZERO_CORE) $(WAIT_ZERO_TEST) tests/test_waitlist_zero.c -o build/test-waitlist-zero
	./build/test-waitlist-zero
sanitize-waitlist-zero:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(WAIT_ZERO_CORE) $(WAIT_ZERO_TEST) tests/test_waitlist_zero.c -o build/test-waitlist-zero-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-waitlist-zero-san
c64-waitlist-zero:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(WAIT_ZERO_CORE) $(WAIT_ZERO_TEST) tools/waitlist_zero_diagnostic.c -Wl,-Map,build/waitlist_zero.map -o build/waitlist_zero.prg
differential-waitlist-zero:
	python3 tools/waitlist_zero_differential.py deps/fixture

all: build/test-waitlist-zero
build/test-waitlist-zero: $(WAIT_ZERO_CORE) $(WAIT_ZERO_TEST) tests/test_waitlist_zero.c tests/waitlist_zero_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WAIT_ZERO_CORE) $(WAIT_ZERO_TEST) tests/test_waitlist_zero.c -o $@
test: test-waitlist-zero
sanitize: sanitize-waitlist-zero
