DSKY_KEYCOM_CORE = src/agc_word.c src/executive.c src/dsky_keycom.c
DSKY_KEYCOM_TEST = tests/dsky_keycom_cases.c tests/dsky_keycom_check.c
.PHONY: test-dsky-keycom sanitize-dsky-keycom c64-dsky-keycom differential-dsky-keycom
test-dsky-keycom:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_KEYCOM_CORE) $(DSKY_KEYCOM_TEST) tests/test_dsky_keycom.c -o build/test-dsky-keycom
	./build/test-dsky-keycom
sanitize-dsky-keycom:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_KEYCOM_CORE) $(DSKY_KEYCOM_TEST) tests/test_dsky_keycom.c -o build/test-dsky-keycom-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-keycom-san
c64-dsky-keycom:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_KEYCOM_CORE) $(DSKY_KEYCOM_TEST) tools/dsky_keycom_diagnostic.c -Wl,-Map,build/dsky_keycom.map -o build/dsky_keycom.prg
differential-dsky-keycom:
	python3 tools/dsky_keycom_differential.py deps/fixture
	python3 tools/freeze_dsky_keycom.py
test: test-dsky-keycom
sanitize: sanitize-dsky-keycom

.PHONY: all
all: c64-dsky-keycom
