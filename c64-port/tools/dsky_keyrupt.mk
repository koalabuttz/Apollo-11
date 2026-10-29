DSKY_KEYRUPT_CORE = src/agc_word.c src/executive.c src/dsky_keycom.c src/dsky_keyrupt.c
DSKY_KEYRUPT_TEST = tests/dsky_keyrupt_cases.c tests/dsky_keyrupt_check.c
.PHONY: test-dsky-keyrupt sanitize-dsky-keyrupt c64-dsky-keyrupt differential-dsky-keyrupt
build/test-dsky-keyrupt: $(DSKY_KEYRUPT_CORE) $(DSKY_KEYRUPT_TEST) tests/test_dsky_keyrupt.c $(wildcard include/*.h tests/*.h) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_KEYRUPT_CORE) $(DSKY_KEYRUPT_TEST) tests/test_dsky_keyrupt.c -o build/test-dsky-keyrupt
test-dsky-keyrupt: build/test-dsky-keyrupt
	./build/test-dsky-keyrupt
sanitize-dsky-keyrupt:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_KEYRUPT_CORE) $(DSKY_KEYRUPT_TEST) tests/test_dsky_keyrupt.c -o build/test-dsky-keyrupt-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-keyrupt-san
c64-dsky-keyrupt:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_KEYRUPT_CORE) $(DSKY_KEYRUPT_TEST) tools/dsky_keyrupt_diagnostic.c -Wl,-Map,build/dsky_keyrupt.map -o build/dsky_keyrupt.prg
differential-dsky-keyrupt:
	python3 tools/dsky_keyrupt_differential.py deps/fixture
	python3 tools/freeze_dsky_keyrupt.py
test: test-dsky-keyrupt
sanitize: sanitize-dsky-keyrupt

.PHONY: all
all: build/test-dsky-keyrupt
