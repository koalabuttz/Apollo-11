DSKY_DISPLAY_CORE = src/agc_word.c src/agc_multiword.c src/p66_mode_display.c src/dsky_relay.c src/dsky_display.c
DSKY_DISPLAY_TEST = tests/dsky_display_check.c
.PHONY: test-dsky-display sanitize-dsky-display c64-dsky-display
test-dsky-display:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DISPLAY_CORE) $(DSKY_DISPLAY_TEST) tests/test_dsky_display.c -o build/test-dsky-display
	./build/test-dsky-display
sanitize-dsky-display:
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_DISPLAY_CORE) $(DSKY_DISPLAY_TEST) tests/test_dsky_display.c -o build/test-dsky-display-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-display-san
c64-dsky-display:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_DISPLAY_CORE) $(DSKY_DISPLAY_TEST) platform/c64/dsky_display.c tools/dsky_display_diagnostic.c -Wl,-Map,build/dsky_display.map -o build/dsky_display.prg
test: test-dsky-display
sanitize: sanitize-dsky-display
