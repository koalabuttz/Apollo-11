P66_MODE_ZERO_CORE = src/agc_word.c src/agc_multiword.c src/p66_mode_display.c
P66_MODE_ZERO_TEST = tests/p66_bias_cases.c tests/p66_mode_display_cases.c tests/p66_mode_zero_cases.c tests/p66_mode_zero_check.c
.PHONY: test-p66-mode-zero sanitize-p66-mode-zero c64-p66-mode-zero differential-p66-mode-zero
test-p66-mode-zero:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_MODE_ZERO_CORE) $(P66_MODE_ZERO_TEST) tests/test_p66_mode_zero.c -o build/test-p66-mode-zero
	./build/test-p66-mode-zero
sanitize-p66-mode-zero:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(P66_MODE_ZERO_CORE) $(P66_MODE_ZERO_TEST) tests/test_p66_mode_zero.c -o build/test-p66-mode-zero-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-mode-zero-san
c64-p66-mode-zero:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(P66_MODE_ZERO_CORE) $(P66_MODE_ZERO_TEST) tools/p66_mode_zero_diagnostic.c -Wl,-Map,build/p66_mode_zero.map -o build/p66_mode_zero.prg
differential-p66-mode-zero:
	python3 tools/p66_mode_zero_differential.py deps/fixture
	python3 tools/freeze_p66_mode_zero.py
test: test-p66-mode-zero
sanitize: sanitize-p66-mode-zero
