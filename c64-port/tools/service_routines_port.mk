# SPDX-License-Identifier: MIT
CC ?= cc
MOS_CC ?= mos-c64-clang
VICE ?= x64sc
ROM_DIR ?= /workspace/toolchains/roms/C64
VICE_DATA_DIR ?= /workspace/toolchains/vice/usr/share/vice
SERVICE_CORE = tests/service_routines_target.c tests/service_routines_cases.c tests/frozen_words.c src/service_routines.c src/agc_word.c
SERVICE_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
.PHONY: all test sanitize check-host vice-test
all: test
test:
	$(CC) -O2 $(SERVICE_FLAGS) tests/test_service_routines.c $(SERVICE_CORE) -o build/test-service-routines
	./build/test-service-routines
sanitize:
	$(CC) -O1 -g $(SERVICE_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer tests/test_service_routines.c $(SERVICE_CORE) -o build/test-service-routines-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-service-routines-sanitize
check-host:
	python3 tools/service_routines_host_test.py
vice-test:
	python3 tools/service_routines_vice_test.py --cc $(MOS_CC) --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
