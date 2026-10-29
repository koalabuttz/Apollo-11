# SPDX-License-Identifier: MIT
STANDARD_LEAD_CORE = src/standard_lead.c src/standard_lead_interp.c src/standard_lead_bank.c src/standard_lead_wake.c src/standard_lead_cm.c src/cm_interbank_extended.c src/lm_basic_wake.c src/executive.c src/cm_basic_wake.c src/cm_native_image.c src/cm_dispatch.c src/cm_executive.c src/agc_word.c
STANDARD_LEAD_CASES = tests/standard_lead_cases.c tests/standard_lead_bank_cases.c tests/standard_lead_interp_cases.c tests/standard_lead_wake_cases.c tests/standard_lead_cm_cases.c tests/standard_lead_resume_cases.c
CC ?= cc
MOS_CC ?= mos-c64-clang
VICE ?= x64sc
ROM_DIR ?= /workspace/toolchains/roms/C64
VICE_DATA_DIR ?= /workspace/toolchains/vice/usr/share/vice
.PHONY: all test sanitize check-host c64 vice-test
all: test
build/test-standard-lead: tests/test_standard_lead.c tests/standard_lead_target_input.c $(STANDARD_LEAD_CORE) $(STANDARD_LEAD_CASES)
	$(CC) -O2 -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -ffunction-sections -Wl,--gc-sections -Iinclude -Itests $^ -o $@
test: build/test-standard-lead
	./build/test-standard-lead
sanitize:
	$(CC) -O1 -g -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude -Itests tests/test_standard_lead.c tests/standard_lead_target_input.c $(STANDARD_LEAD_CORE) $(STANDARD_LEAD_CASES) -o build/test-standard-lead-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-standard-lead-sanitize
check-host:
	python3 tools/standard_lead_host_test.py
c64 vice-test:
	python3 tools/standard_lead_vice_test.py --cc $(MOS_CC) --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
