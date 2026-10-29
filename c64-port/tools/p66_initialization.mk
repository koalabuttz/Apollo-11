P66_INIT_CORE = src/p66_initialization.c
P66_INIT_TEST = tests/p66_initialization_cases.c tests/p66_initialization_check.c
.PHONY: test-p66-initialization sanitize-p66-initialization c64-p66-initialization differential-p66-initialization
test-p66-initialization:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_INIT_CORE) $(P66_INIT_TEST) tests/test_p66_initialization.c -o build/test-p66-initialization
	./build/test-p66-initialization
sanitize-p66-initialization:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(P66_INIT_CORE) $(P66_INIT_TEST) tests/test_p66_initialization.c -o build/test-p66-initialization-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-initialization-san
c64-p66-initialization:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(P66_INIT_CORE) $(P66_INIT_TEST) tools/p66_initialization_diagnostic.c -Wl,-Map,build/p66_initialization.map -o build/p66_initialization.prg
differential-p66-initialization:
	python3 tools/p66_initialization_differential.py deps/fixture

test: test-p66-initialization
sanitize: sanitize-p66-initialization
