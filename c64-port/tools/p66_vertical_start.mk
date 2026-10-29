P66_VSTART_CORE = src/agc_word.c src/executive.c src/waitlist.c src/phase.c src/p66_startup.c src/p66_initialization.c src/p66_vertical_start.c
P66_VSTART_TEST = tests/p66_bias_cases.c tests/p66_vertical_start_cases.c tests/p66_vertical_start_check.c
.PHONY: test-p66-vertical-start sanitize-p66-vertical-start c64-p66-vertical-start differential-p66-vertical-start
test-p66-vertical-start:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_VSTART_CORE) $(P66_VSTART_TEST) tests/test_p66_vertical_start.c -o build/test-p66-vertical-start
	./build/test-p66-vertical-start
sanitize-p66-vertical-start:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(P66_VSTART_CORE) $(P66_VSTART_TEST) tests/test_p66_vertical_start.c -o build/test-p66-vertical-start-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-vertical-start-san
c64-p66-vertical-start:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(P66_VSTART_CORE) $(P66_VSTART_TEST) tools/p66_vertical_start_diagnostic.c -Wl,-Map,build/p66_vertical_start.map -o build/p66_vertical_start.prg
differential-p66-vertical-start:
	python3 tools/p66_vertical_start_differential.py deps/fixture
	python3 tools/freeze_p66_vertical_start.py

test: test-p66-vertical-start
sanitize: sanitize-p66-vertical-start
