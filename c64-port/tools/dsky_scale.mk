DSKY_SCALE_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/dsky_scale.c
DSKY_SCALE_TEST = tests/p66_bias_cases.c tests/dsky_scale_cases.c tests/dsky_scale_check.c
.PHONY: test-dsky-scale sanitize-dsky-scale c64-dsky-scale differential-dsky-scale
test-dsky-scale:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_SCALE_CORE) $(DSKY_SCALE_TEST) tests/test_dsky_scale.c -o build/test-dsky-scale
	./build/test-dsky-scale
sanitize-dsky-scale:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_SCALE_CORE) $(DSKY_SCALE_TEST) tests/test_dsky_scale.c -o build/test-dsky-scale-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-scale-san
c64-dsky-scale:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_SCALE_CORE) $(DSKY_SCALE_TEST) tools/dsky_scale_diagnostic.c -Wl,-Map,build/dsky_scale.map -o build/dsky_scale.prg
differential-dsky-scale:
	python3 tools/dsky_scale_differential.py deps/fixture
	python3 tools/freeze_dsky_scale.py
test: test-dsky-scale
sanitize: sanitize-dsky-scale
