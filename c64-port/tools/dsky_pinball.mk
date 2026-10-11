DSKY_PINBALL_CORE = src/agc_word.c src/dsky_pinball.c
DSKY_PINBALL_TEST = tests/p66_bias_cases.c tests/dsky_pinball_cases.c tests/dsky_pinball_check.c
.PHONY: test-dsky-pinball sanitize-dsky-pinball c64-dsky-pinball differential-dsky-pinball
test-dsky-pinball:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_PINBALL_CORE) $(DSKY_PINBALL_TEST) tests/test_dsky_pinball.c -o build/test-dsky-pinball
	./build/test-dsky-pinball
sanitize-dsky-pinball:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_PINBALL_CORE) $(DSKY_PINBALL_TEST) tests/test_dsky_pinball.c -o build/test-dsky-pinball-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-pinball-san
c64-dsky-pinball:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_PINBALL_CORE) $(DSKY_PINBALL_TEST) tools/dsky_pinball_diagnostic.c -Wl,-Map,build/dsky_pinball.map -o build/dsky_pinball.prg
differential-dsky-pinball:
	python3 tools/dsky_pinball_differential.py deps/fixture
	python3 tools/freeze_dsky_pinball.py
test: test-dsky-pinball
sanitize: sanitize-dsky-pinball
