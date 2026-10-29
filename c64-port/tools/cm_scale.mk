# SPDX-License-Identifier: MIT
CM_SCALE_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
CM_SCALE_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/dsky_scale.c src/cm_scale.c tests/p66_bias_cases.c tests/dsky_scale_cases.c tests/cm_scale_cases.c
CM_SCALE_TEST = tests/test_cm_scale.c tests/cm_scale_check.c
all: build/test-cm-scale
build/test-cm-scale: $(CM_SCALE_TEST) $(CM_SCALE_CORE) tests/cm_scale_vectors.h
	@mkdir -p build
	$(CC) -O2 -Wconversion -pedantic $(CM_SCALE_FLAGS) $(CM_SCALE_TEST) $(CM_SCALE_CORE) -o $@
test-cm-scale: build/test-cm-scale
	build/test-cm-scale
sanitize-cm-scale:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_SCALE_FLAGS) $(CM_SCALE_TEST) $(CM_SCALE_CORE) -o build/test-cm-scale-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-scale-sanitize
c64-cm-scale:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_SCALE_FLAGS) tools/cm_scale_diagnostic.c tests/cm_scale_check.c $(CM_SCALE_CORE) -Wl,-Map,build/cm_scale.map -o build/cm_scale.prg
differential-cm-scale:
	python3 tools/cm_scale_differential.py
test: test-cm-scale
sanitize: sanitize-cm-scale
.PHONY: test-cm-scale sanitize-cm-scale c64-cm-scale differential-cm-scale
