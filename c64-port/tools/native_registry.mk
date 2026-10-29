# SPDX-License-Identifier: MIT
CC ?= cc
CFLAGS := -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
SOURCES := src/overlay_runtime.c src/native_module_registry.c tests/test_native_module_registry.c
.PHONY: test sanitize target
test:
	$(CC) $(CFLAGS) $(SOURCES) -o build/test-native-module-registry
	build/test-native-module-registry
sanitize:
	$(CC) $(CFLAGS) -g -O1 -no-pie -fsanitize=address,undefined $(SOURCES) -o build/test-native-module-registry-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-native-module-registry-sanitize
target:
	python3 tools/native_registry_build.py --cc $(MOS_CC) --c1541 $(C1541)
