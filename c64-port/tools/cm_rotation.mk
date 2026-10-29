# SPDX-License-Identifier: MIT
CM_ROT_FLAGS = -DROTATION_INPUT_ONLY -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
CM_ROT_INPUT = tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/rotation_cases.c tests/cm_rotation_cases.c
CM_ROT_CORE = src/cm_rotation.c src/guidance_product.c src/agc_word.c src/agc_multiword.c
CM_ROT_HEADERS = include/cm_rotation.h include/guidance_rotation.h include/guidance_product.h $(wildcard include/guidance*.h) tests/cm_rotation_cases.h tests/cm_rotation_vectors.h tests/rotation_cases.h tests/frozen_words.h
all: build/cm-rotation-native build/test-cm-rotation
build/cm-rotation-native: $(CM_ROT_CORE) $(CM_ROT_INPUT) platform/host/cm_rotation_snapshot.c $(CM_ROT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ROT_FLAGS) $(CM_ROT_CORE) $(CM_ROT_INPUT) platform/host/cm_rotation_snapshot.c -o $@
build/test-cm-rotation: $(CM_ROT_CORE) $(CM_ROT_INPUT) tests/test_cm_rotation.c tests/frozen_words.c $(CM_ROT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ROT_FLAGS) $(CM_ROT_CORE) $(CM_ROT_INPUT) tests/test_cm_rotation.c tests/frozen_words.c -o $@
.PHONY: cm-rotation-differential test-cm-rotation sanitize-cm-rotation c64-cm-rotation
cm-rotation-differential: build/cm-rotation-native
	python3 tools/cm_rotation_differential.py
test: test-cm-rotation
test-cm-rotation: build/test-cm-rotation
	./build/test-cm-rotation
sanitize: sanitize-cm-rotation
sanitize-cm-rotation: | build
	$(CC) $(CPPFLAGS) $(CM_ROT_FLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ROT_CORE) $(CM_ROT_INPUT) tests/test_cm_rotation.c tests/frozen_words.c -o build/test-cm-rotation-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-rotation-sanitize
c64-cm-rotation: build/cm-rotation.prg
build/cm-rotation.prg: $(CM_ROT_CORE) $(CM_ROT_INPUT) tests/test_cm_rotation.c tests/frozen_words.c platform/c64/cm_rotation_diagnostic.c $(CM_ROT_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_ROT_FLAGS) -DCM_ROTATION_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-rotation.map $(CM_ROT_CORE) $(CM_ROT_INPUT) tests/test_cm_rotation.c tests/frozen_words.c platform/c64/cm_rotation_diagnostic.c -o $@
