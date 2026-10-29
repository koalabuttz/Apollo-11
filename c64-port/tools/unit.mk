# SPDX-License-Identifier: MIT
UNIT_CORE = src/agc_word.c src/agc_multiword.c src/guidance_dp_divide.c src/guidance_norm.c src/guidance_unit.c
UNIT_INPUT = tests/scenarios.c tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c
UNIT_DEFINES = -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
UNIT_HEADERS = include/guidance_unit.h include/guidance_norm.h include/guidance_dp_divide.h tests/unit_cases.h tests/unit_vectors.h tests/frozen_words.h
all: build/unit-native build/test-unit
build/unit-native: $(UNIT_CORE) $(UNIT_INPUT) tests/unit_snapshot.c $(UNIT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(UNIT_DEFINES) $(UNIT_CORE) $(UNIT_INPUT) tests/unit_snapshot.c -o $@
build/test-unit: $(UNIT_CORE) $(UNIT_INPUT) tests/test_unit.c tests/frozen_words.c $(UNIT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(UNIT_DEFINES) $(UNIT_CORE) $(UNIT_INPUT) tests/test_unit.c tests/frozen_words.c -o $@
test: test-unit
.PHONY: test-unit sanitize-unit c64-unit
test-unit: build/test-unit
	./build/test-unit
sanitize: sanitize-unit
sanitize-unit: | build
	$(CC) $(CPPFLAGS) $(UNIT_DEFINES) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(UNIT_CORE) $(UNIT_INPUT) tests/test_unit.c tests/frozen_words.c -o build/test-unit-sanitize
	./build/test-unit-sanitize
c64-unit: build/unit.prg
build/unit.prg: $(UNIT_CORE) $(UNIT_INPUT) tests/test_unit.c tests/frozen_words.c tools/unit_diagnostic.c $(UNIT_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) $(UNIT_DEFINES) -DUNIT_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/unit.map $(UNIT_CORE) $(UNIT_INPUT) tests/test_unit.c tests/frozen_words.c tools/unit_diagnostic.c -o $@
