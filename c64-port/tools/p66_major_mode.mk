P66_MAJOR_CORE = src/agc_word.c src/executive.c src/p66_major_mode.c
P66_MAJOR_TEST = tests/p66_bias_cases.c tests/p66_major_mode_cases.c tests/p66_major_mode_check.c
.PHONY: test-p66-major-mode sanitize-p66-major-mode c64-p66-major-mode differential-p66-major-mode
test-p66-major-mode:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_MAJOR_CORE) $(P66_MAJOR_TEST) tests/test_p66_major_mode.c -o build/test-p66-major-mode
	./build/test-p66-major-mode
sanitize-p66-major-mode:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(P66_MAJOR_CORE) $(P66_MAJOR_TEST) tests/test_p66_major_mode.c -o build/test-p66-major-mode-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-major-mode-san
c64-p66-major-mode:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(P66_MAJOR_CORE) $(P66_MAJOR_TEST) tools/p66_major_mode_diagnostic.c -Wl,-Map,build/p66_major_mode.map -o build/p66_major_mode.prg
differential-p66-major-mode:
	python3 tools/p66_major_mode_differential.py deps/fixture
	python3 tools/freeze_p66_major_mode.py
test: test-p66-major-mode
sanitize: sanitize-p66-major-mode
