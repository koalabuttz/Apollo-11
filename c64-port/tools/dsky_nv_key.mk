DSKY_NV_KEY_CORE = src/agc_word.c  src/dsky_pinball.c src/dsky_nv_key.c
DSKY_NV_KEY_TEST = tests/p66_bias_cases.c tests/dsky_nv_key_cases.c tests/dsky_nv_key_check.c
.PHONY: test-dsky-nv-key sanitize-dsky-nv-key c64-dsky-nv-key differential-dsky-nv-key
test-dsky-nv-key:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_KEY_CORE) $(DSKY_NV_KEY_TEST) tests/test_dsky_nv_key.c -o build/test-dsky-nv-key
	./build/test-dsky-nv-key
sanitize-dsky-nv-key:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_NV_KEY_CORE) $(DSKY_NV_KEY_TEST) tests/test_dsky_nv_key.c -o build/test-dsky-nv-key-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-nv-key-san
c64-dsky-nv-key:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_NV_KEY_CORE) $(DSKY_NV_KEY_TEST) tools/dsky_nv_key_diagnostic.c -Wl,-Map,build/dsky_nv_key.map -o build/dsky_nv_key.prg
differential-dsky-nv-key:
	python3 tools/dsky_nv_key_differential.py deps/fixture
	python3 tools/freeze_dsky_nv_key.py
test: test-dsky-nv-key
sanitize: sanitize-dsky-nv-key

.PHONY: all
all: c64-dsky-nv-key
