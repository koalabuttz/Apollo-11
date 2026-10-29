P66_START_CORE = src/agc_word.c src/executive.c src/waitlist.c src/phase.c src/p66_startup.c
P66_START_TEST = tests/p66_startup_cases.c tests/p66_startup_check.c
.PHONY: test-p66-startup sanitize-p66-startup c64-p66-startup differential-p66-startup
test-p66-startup:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_START_CORE) $(P66_START_TEST) tests/test_p66_startup.c -o build/test-p66-startup
	./build/test-p66-startup
sanitize-p66-startup:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(P66_START_CORE) $(P66_START_TEST) tests/test_p66_startup.c -o build/test-p66-startup-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-startup-san
c64-p66-startup:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(P66_START_CORE) $(P66_START_TEST) tools/p66_startup_diagnostic.c -Wl,-Map,build/p66_startup.map -o build/p66_startup.prg
differential-p66-startup:
	python3 tools/p66_startup_differential.py deps/fixture
	python3 tools/freeze_p66_startup.py

all: build/test-p66-startup
build/test-p66-startup: $(P66_START_CORE) $(P66_START_TEST) tests/test_p66_startup.c tests/p66_startup_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_START_CORE) $(P66_START_TEST) tests/test_p66_startup.c -o $@
test: test-p66-startup
sanitize: sanitize-p66-startup
