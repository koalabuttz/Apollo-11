# SPDX-License-Identifier: MIT
LM_INTERBANK_INTEGRATION_CORE = tests/lm_interbank_target.c tests/lm_interbank_cases.c tests/frozen_words.c src/lm_interbank.c src/agc_word.c
all: build/test-lm-interbank-integrated
build/test-lm-interbank-integrated: tests/test_lm_interbank.c $(LM_INTERBANK_INTEGRATION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
test: test-lm-interbank-integrated
test-lm-interbank-integrated: build/test-lm-interbank-integrated
	./build/test-lm-interbank-integrated
sanitize: sanitize-lm-interbank-integrated
sanitize-lm-interbank-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_lm_interbank.c $(LM_INTERBANK_INTEGRATION_CORE) -o build/test-lm-interbank-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-interbank-integrated-san
.PHONY: test-lm-interbank-integrated sanitize-lm-interbank-integrated
