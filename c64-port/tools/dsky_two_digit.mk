DSKY_TWO_DIGIT_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_two_digit.c
DSKY_TWO_DIGIT_TEST = tests/p66_bias_cases.c tests/dsky_two_digit_cases.c tests/dsky_two_digit_check.c
.PHONY: test-dsky-two-digit sanitize-dsky-two-digit c64-dsky-two-digit differential-dsky-two-digit
test-dsky-two-digit:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_TWO_DIGIT_CORE) $(DSKY_TWO_DIGIT_TEST) tests/test_dsky_two_digit.c -o build/test-dsky-two-digit
	./build/test-dsky-two-digit
sanitize-dsky-two-digit:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_TWO_DIGIT_CORE) $(DSKY_TWO_DIGIT_TEST) tests/test_dsky_two_digit.c -o build/test-dsky-two-digit-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-two-digit-san
c64-dsky-two-digit:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_TWO_DIGIT_CORE) $(DSKY_TWO_DIGIT_TEST) tools/dsky_two_digit_diagnostic.c -Wl,-Map,build/dsky_two_digit.map -o build/dsky_two_digit.prg
differential-dsky-two-digit:
	python3 tools/dsky_two_digit_differential.py deps/fixture
	python3 tools/freeze_dsky_two_digit.py
test: test-dsky-two-digit
sanitize: sanitize-dsky-two-digit
