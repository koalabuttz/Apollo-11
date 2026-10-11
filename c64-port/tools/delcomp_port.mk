# SPDX-License-Identifier: MIT
DELCOMP_PORT_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/guidance_shift.c src/guidance_sum.c src/guidance_trig.c src/delcomp.c tests/delcomp_cases.c
all: build/test-delcomp-port
build/test-delcomp-port: $(DELCOMP_PORT_CORE) tests/test_delcomp.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_delcomp.c $(DELCOMP_PORT_CORE) -o $@
test: test-delcomp-port
test-delcomp-port: build/test-delcomp-port
	./build/test-delcomp-port
sanitize: sanitize-delcomp-port
sanitize-delcomp-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_delcomp.c $(DELCOMP_PORT_CORE) -o build/test-delcomp-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-delcomp-port-sanitize
.PHONY: test-delcomp-port sanitize-delcomp-port
