# SPDX-License-Identifier: MIT
CM_NUM_COMMON = src/agc_word.c src/agc_multiword.c
CM_NUM_INPUT = tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c
CM_NUM_FLAGS = -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
CM_NUM_HEADERS = $(wildcard include/guidance*.h) include/agc_word.h include/agc_multiword.h tests/frozen_words.h tests/norm_cases.h tests/norm_vectors.h tests/dp_divide_cases.h tests/dp_divide_vectors.h tests/unit_cases.h tests/unit_vectors.h
.PHONY: cm-numeric-differential test-cm-numeric sanitize-cm-numeric c64-cm-numeric
all: build/test-cm-norm build/test-cm-dp-divide build/test-cm-unit
cm-numeric-differential: | build
	python3 tools/cm_numeric_differential.py
test: test-cm-numeric
sanitize: sanitize-cm-numeric
test-cm-numeric: test-cm-norm test-cm-dp-divide test-cm-unit
sanitize-cm-numeric: sanitize-cm-norm sanitize-cm-dp-divide sanitize-cm-unit
c64-cm-numeric: c64-cm-norm c64-cm-dp-divide c64-cm-unit

.PHONY: test-cm-norm sanitize-cm-norm c64-cm-norm
build/test-cm-norm: $(CM_NUM_COMMON) src/guidance_norm.c  tests/test_norm.c tests/frozen_words.c $(CM_NUM_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS)  $(CM_NUM_COMMON) src/guidance_norm.c  tests/test_norm.c tests/frozen_words.c -o $@
test-cm-norm: build/test-cm-norm
	./build/test-cm-norm
sanitize-cm-norm: | build
	$(CC) $(CPPFLAGS)  -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_NUM_COMMON) src/guidance_norm.c  tests/test_norm.c tests/frozen_words.c -o build/test-cm-norm-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-norm-sanitize
c64-cm-norm: build/cm-norm.prg
build/cm-norm.prg: $(CM_NUM_COMMON) src/guidance_norm.c  tests/test_norm.c tests/frozen_words.c tools/norm_diagnostic.c $(CM_NUM_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS)  -DNORM_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-norm.map $(CM_NUM_COMMON) src/guidance_norm.c  tests/test_norm.c tests/frozen_words.c tools/norm_diagnostic.c -o $@

.PHONY: test-cm-dp-divide sanitize-cm-dp-divide c64-cm-dp-divide
build/test-cm-dp-divide: $(CM_NUM_COMMON) src/guidance_dp_divide.c  tests/test_dp_divide.c tests/frozen_words.c $(CM_NUM_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS)  $(CM_NUM_COMMON) src/guidance_dp_divide.c  tests/test_dp_divide.c tests/frozen_words.c -o $@
test-cm-dp-divide: build/test-cm-dp-divide
	./build/test-cm-dp-divide
sanitize-cm-dp-divide: | build
	$(CC) $(CPPFLAGS)  -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_NUM_COMMON) src/guidance_dp_divide.c  tests/test_dp_divide.c tests/frozen_words.c -o build/test-cm-dp-divide-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-dp-divide-sanitize
c64-cm-dp-divide: build/cm-dp-divide.prg
build/cm-dp-divide.prg: $(CM_NUM_COMMON) src/guidance_dp_divide.c  tests/test_dp_divide.c tests/frozen_words.c tools/dp_divide_diagnostic.c $(CM_NUM_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS)  -DDP_DIVIDE_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-dp-divide.map $(CM_NUM_COMMON) src/guidance_dp_divide.c  tests/test_dp_divide.c tests/frozen_words.c tools/dp_divide_diagnostic.c -o $@

.PHONY: test-cm-unit sanitize-cm-unit c64-cm-unit
build/test-cm-unit: $(CM_NUM_COMMON) src/guidance_unit.c src/guidance_norm.c src/guidance_dp_divide.c $(CM_NUM_INPUT) tests/test_unit.c tests/frozen_words.c $(CM_NUM_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_NUM_FLAGS) $(CM_NUM_COMMON) src/guidance_unit.c src/guidance_norm.c src/guidance_dp_divide.c $(CM_NUM_INPUT) tests/test_unit.c tests/frozen_words.c -o $@
test-cm-unit: build/test-cm-unit
	./build/test-cm-unit
sanitize-cm-unit: | build
	$(CC) $(CPPFLAGS) $(CM_NUM_FLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_NUM_COMMON) src/guidance_unit.c src/guidance_norm.c src/guidance_dp_divide.c $(CM_NUM_INPUT) tests/test_unit.c tests/frozen_words.c -o build/test-cm-unit-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-unit-sanitize
c64-cm-unit: build/cm-unit.prg
build/cm-unit.prg: $(CM_NUM_COMMON) src/guidance_unit.c src/guidance_norm.c src/guidance_dp_divide.c $(CM_NUM_INPUT) tests/test_unit.c tests/frozen_words.c tools/unit_diagnostic.c $(CM_NUM_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_NUM_FLAGS) -DUNIT_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-unit.map $(CM_NUM_COMMON) src/guidance_unit.c src/guidance_norm.c src/guidance_dp_divide.c $(CM_NUM_INPUT) tests/test_unit.c tests/frozen_words.c tools/unit_diagnostic.c -o $@
