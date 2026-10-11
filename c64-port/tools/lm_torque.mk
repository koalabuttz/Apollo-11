# SPDX-License-Identifier: MIT
LM_TORQUE_CORE = src/agc_word.c src/agc_multiword.c src/lm_torque.c tests/lm_torque_cases.c tests/test_lm_torque.c
LM_TORQUE_HEADERS = include/lm_torque.h tests/lm_torque_vectors.h
all: build/test-lm-torque
build/test-lm-torque: $(LM_TORQUE_CORE) $(LM_TORQUE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_TORQUE_CORE) -o $@
test: test-lm-torque
test-lm-torque: build/test-lm-torque
	./build/test-lm-torque
sanitize: sanitize-lm-torque
sanitize-lm-torque: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_TORQUE_CORE) -o build/test-lm-torque-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-torque-sanitize
.PHONY: test-lm-torque sanitize-lm-torque
