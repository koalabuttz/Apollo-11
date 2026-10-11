DSKY_PINBALL_ZERO_CORE = src/agc_word.c src/dsky_pinball.c
DSKY_PINBALL_ZERO_TEST = tests/p66_bias_cases.c tests/dsky_pinball_cases.c tests/dsky_pinball_zero_cases.c tests/dsky_pinball_zero_check.c
.PHONY: test-dsky-pinball-zero sanitize-dsky-pinball-zero c64-dsky-pinball-zero differential-dsky-pinball-zero
test-dsky-pinball-zero:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_PINBALL_ZERO_CORE) $(DSKY_PINBALL_ZERO_TEST) tests/test_dsky_pinball_zero.c -o build/test-dsky-pinball-zero
	./build/test-dsky-pinball-zero
sanitize-dsky-pinball-zero:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_PINBALL_ZERO_CORE) $(DSKY_PINBALL_ZERO_TEST) tests/test_dsky_pinball_zero.c -o build/test-dsky-pinball-zero-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-pinball-zero-san
c64-dsky-pinball-zero:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_PINBALL_ZERO_CORE) $(DSKY_PINBALL_ZERO_TEST) tools/dsky_pinball_zero_diagnostic.c -Wl,-Map,build/dsky_pinball_zero.map -o build/dsky_pinball_zero.prg
differential-dsky-pinball-zero:
	python3 tools/dsky_pinball_zero_differential.py deps/fixture
	python3 tools/freeze_dsky_pinball_zero.py
test: test-dsky-pinball-zero
sanitize: sanitize-dsky-pinball-zero
