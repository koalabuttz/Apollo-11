# SPDX-License-Identifier: MIT
GIMBAL_PORT_CORE = src/agc_word.c src/agc_multiword.c src/guidance_shift.c src/gimbal_stage.c tests/gimbal_cases.c
all: build/test-gimbal-port
build/test-gimbal-port: $(GIMBAL_PORT_CORE) tests/test_gimbal.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_gimbal.c $(GIMBAL_PORT_CORE) -o $@
test: test-gimbal-port
test-gimbal-port: build/test-gimbal-port
	./build/test-gimbal-port
sanitize: sanitize-gimbal-port
sanitize-gimbal-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_gimbal.c $(GIMBAL_PORT_CORE) -o build/test-gimbal-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-gimbal-port-sanitize
.PHONY: test-gimbal-port sanitize-gimbal-port
