# SPDX-License-Identifier: MIT
# The trampoline's fast paths (lane D2) against the slow forms they replace.
AGC_TF_SRC = src/agc_transfer.c src/agc_word.c
.PHONY: test-agc-transfer-fast sanitize-agc-transfer-fast
all: build/test-agc-transfer-fast
test: test-agc-transfer-fast
sanitize: sanitize-agc-transfer-fast
build/test-agc-transfer-fast: $(AGC_TF_SRC) tests/test_agc_transfer_fast.c include/agc_transfer.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(AGC_TF_SRC) tests/test_agc_transfer_fast.c -o $@
test-agc-transfer-fast: build/test-agc-transfer-fast
	./build/test-agc-transfer-fast
sanitize-agc-transfer-fast: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(AGC_TF_SRC) tests/test_agc_transfer_fast.c -o build/test-agc-transfer-fast-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-agc-transfer-fast-sanitize
