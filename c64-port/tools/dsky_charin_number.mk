CPPFLAGS ?= -Iinclude -Itests
CFLAGS ?= -O2 -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic
MOS_CC ?= mos-c64-clang
DSKY_CHARIN_NUMBER_CORE = src/agc_word.c  src/dsky_pinball.c src/agc_multiword.c src/dsky_charin_number.c
DSKY_CHARIN_NUMBER_TEST = tests/p66_bias_cases.c tests/dsky_nv_key_cases.c tests/dsky_charin_number_cases.c tests/dsky_charin_number_check.c
.PHONY: test-dsky-charin-number sanitize-dsky-charin-number c64-dsky-charin-number differential-dsky-charin-number
build/test-dsky-charin-number: $(DSKY_CHARIN_NUMBER_CORE) $(DSKY_CHARIN_NUMBER_TEST) tests/test_dsky_charin_number.c $(wildcard include/*.h tests/*.h)
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_CHARIN_NUMBER_CORE) $(DSKY_CHARIN_NUMBER_TEST) tests/test_dsky_charin_number.c -o build/test-dsky-charin-number
test-dsky-charin-number: build/test-dsky-charin-number
	./build/test-dsky-charin-number
sanitize-dsky-charin-number:
	@mkdir -p build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_CHARIN_NUMBER_CORE) $(DSKY_CHARIN_NUMBER_TEST) tests/test_dsky_charin_number.c -o build/test-dsky-charin-number-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-charin-number-san
c64-dsky-charin-number:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_CHARIN_NUMBER_CORE) $(DSKY_CHARIN_NUMBER_TEST) tools/dsky_charin_number_diagnostic.c -Wl,-Map,build/dsky_charin_number.map -o build/dsky_charin_number.prg
differential-dsky-charin-number:
	python3 tools/dsky_charin_number_differential.py deps/fixture
	python3 tools/freeze_dsky_charin_number.py
test: test-dsky-charin-number
sanitize: sanitize-dsky-charin-number

.PHONY: all
all: build/test-dsky-charin-number
