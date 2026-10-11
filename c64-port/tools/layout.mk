# SPDX-License-Identifier: MIT
LAYOUT_CORE = src/layout.c tests/layout_check.c tests/test_layout.c
LAYOUT_HEADERS = include/layout.h include/fixed_data_symbols.h src/layout_data.inc tests/layout_vectors.h
all: build/test-layout
build/test-layout: $(LAYOUT_CORE) $(LAYOUT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LAYOUT_CORE) -o $@
test: test-layout
test-layout: build/test-layout
	./build/test-layout
sanitize: sanitize-layout
sanitize-layout: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LAYOUT_CORE) -o build/test-layout-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-layout-sanitize
.PHONY: test-layout sanitize-layout
