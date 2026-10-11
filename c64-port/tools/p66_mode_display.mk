P66_MODE_CORE = src/agc_word.c src/agc_multiword.c src/p66_mode_display.c
P66_MODE_TEST = tests/p66_bias_cases.c tests/p66_mode_display_cases.c tests/p66_mode_display_check.c
.PHONY: test-p66-mode-display sanitize-p66-mode-display c64-p66-mode-display differential-p66-mode-display
test-p66-mode-display:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_MODE_CORE) $(P66_MODE_TEST) tests/test_p66_mode_display.c -o build/test-p66-mode-display
	./build/test-p66-mode-display
sanitize-p66-mode-display:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(P66_MODE_CORE) $(P66_MODE_TEST) tests/test_p66_mode_display.c -o build/test-p66-mode-display-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-mode-display-san
c64-p66-mode-display:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(P66_MODE_CORE) $(P66_MODE_TEST) tools/p66_mode_display_diagnostic.c -Wl,-Map,build/p66_mode_display.map -o build/p66_mode_display.prg
differential-p66-mode-display:
	python3 tools/p66_mode_display_differential.py deps/fixture
	python3 tools/freeze_p66_mode_display.py
test: test-p66-mode-display
sanitize: sanitize-p66-mode-display
