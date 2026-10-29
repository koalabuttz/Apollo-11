DSKY_CHARIN_NV_CORE = src/agc_word.c  src/dsky_pinball.c src/dsky_nv_key.c src/dsky_charin_nv.c
DSKY_CHARIN_NV_TEST = tests/p66_bias_cases.c tests/dsky_nv_key_cases.c tests/dsky_charin_nv_cases.c tests/dsky_charin_nv_check.c
.PHONY: test-dsky-charin-nv sanitize-dsky-charin-nv c64-dsky-charin-nv differential-dsky-charin-nv
build/test-dsky-charin-nv: $(DSKY_CHARIN_NV_CORE) $(DSKY_CHARIN_NV_TEST) tests/test_dsky_charin_nv.c $(wildcard include/*.h tests/*.h) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_CHARIN_NV_CORE) $(DSKY_CHARIN_NV_TEST) tests/test_dsky_charin_nv.c -o build/test-dsky-charin-nv
test-dsky-charin-nv: build/test-dsky-charin-nv
	./build/test-dsky-charin-nv
sanitize-dsky-charin-nv:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_CHARIN_NV_CORE) $(DSKY_CHARIN_NV_TEST) tests/test_dsky_charin_nv.c -o build/test-dsky-charin-nv-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-charin-nv-san
c64-dsky-charin-nv:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_CHARIN_NV_CORE) $(DSKY_CHARIN_NV_TEST) tools/dsky_charin_nv_diagnostic.c -Wl,-Map,build/dsky_charin_nv.map -o build/dsky_charin_nv.prg
differential-dsky-charin-nv:
	python3 tools/dsky_charin_nv_differential.py deps/fixture
	python3 tools/freeze_dsky_charin_nv.py
test: test-dsky-charin-nv
sanitize: sanitize-dsky-charin-nv

.PHONY: all
all: build/test-dsky-charin-nv
