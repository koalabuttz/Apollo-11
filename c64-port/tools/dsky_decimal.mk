DSKY_DECIMAL_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_decimal.c
DSKY_DECIMAL_TEST = tests/p66_bias_cases.c tests/dsky_decimal_cases.c tests/dsky_decimal_check.c
.PHONY: test-dsky-decimal sanitize-dsky-decimal c64-dsky-decimal differential-dsky-decimal
test-dsky-decimal:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_CORE) $(DSKY_DECIMAL_TEST) tests/test_dsky_decimal.c -o build/test-dsky-decimal
	./build/test-dsky-decimal
sanitize-dsky-decimal:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_DECIMAL_CORE) $(DSKY_DECIMAL_TEST) tests/test_dsky_decimal.c -o build/test-dsky-decimal-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-decimal-san
c64-dsky-decimal:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_DECIMAL_CORE) $(DSKY_DECIMAL_TEST) tools/dsky_decimal_diagnostic.c -Wl,-Map,build/dsky_decimal.map -o build/dsky_decimal.prg
differential-dsky-decimal:
	python3 tools/dsky_decimal_differential.py deps/fixture
	python3 tools/freeze_dsky_decimal.py
test: test-dsky-decimal
sanitize: sanitize-dsky-decimal
