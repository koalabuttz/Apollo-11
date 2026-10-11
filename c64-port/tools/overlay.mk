# SPDX-License-Identifier: MIT
C1541 ?= c1541
OVERLAY_CORE = src/overlay_runtime.c
OVERLAY_TEST = tests/test_overlay.c
OVERLAY_HEADERS = include/overlay_runtime.h
all: build/test-overlay
build/test-overlay: $(OVERLAY_CORE) $(OVERLAY_TEST) $(OVERLAY_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -Wconversion -pedantic $(OVERLAY_CORE) $(OVERLAY_TEST) -o $@
test: test-overlay
.PHONY: test-overlay sanitize-overlay c64-overlay overlay-inventory
test-overlay: build/test-overlay
	./build/test-overlay
sanitize: sanitize-overlay
sanitize-overlay: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(OVERLAY_CORE) $(OVERLAY_TEST) -o build/test-overlay-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-overlay-sanitize
c64-overlay: | build
	python3 tools/overlay_build.py --cc $(MOS_CC) --c1541 $(C1541)
overlay-inventory: | build
	python3 tools/overlay_source_manifest.py
