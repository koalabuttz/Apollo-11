P66_BIAS_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/p66_bias.c
P66_BIAS_TEST = tests/p66_bias_cases.c tests/p66_bias_check.c
.PHONY: test-p66-bias sanitize-p66-bias c64-p66-bias differential-p66-bias
test-p66-bias:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_BIAS_CORE) $(P66_BIAS_TEST) tests/test_p66_bias.c -o build/test-p66-bias
	./build/test-p66-bias
sanitize-p66-bias:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(P66_BIAS_CORE) $(P66_BIAS_TEST) tests/test_p66_bias.c -o build/test-p66-bias-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-bias-san
c64-p66-bias:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(P66_BIAS_CORE) $(P66_BIAS_TEST) tools/p66_bias_diagnostic.c -Wl,-Map,build/p66_bias.map -o build/p66_bias.prg
differential-p66-bias:
	python3 tools/p66_bias_differential.py deps/fixture
	python3 tools/freeze_p66_bias.py

test: test-p66-bias
sanitize: sanitize-p66-bias
