# SPDX-License-Identifier: MIT
FIXED_DATA_CORE = src/fixed_data.c tests/test_fixed_data.c
all: build/test-fixed-data
build/test-fixed-data: $(FIXED_DATA_CORE) include/fixed_data.h src/fixed_data_lm.inc src/fixed_data_cm.inc | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FIXED_DATA_CORE) -o $@
fixed-data-test: build/test-fixed-data
	./build/test-fixed-data
	python3 tests/test_fixed_data.py
fixed-data-sanitize:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(FIXED_DATA_CORE) -o build/test-fixed-data-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-fixed-data-san
test: fixed-data-test
sanitize: fixed-data-sanitize
.PHONY: fixed-data-test fixed-data-sanitize
