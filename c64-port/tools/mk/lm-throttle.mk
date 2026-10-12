# SPDX-License-Identifier: MIT
# Milestone lm-throttle (lane F): Luminary099 THROTTLE_CONTROL_ROUTINES and
# LUNAR_LANDING_GUIDANCE_EQUATIONS FASTCHNG as native routines of the
# control-transfer ABI.
LM_THR_SRC = src/lm_throttle.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_throttle_cases.c
LM_THR_HDR = include/lm_throttle.h include/lm_flight_io.h include/agc_image_ops.h include/agc_transfer.h tests/lm_throttle_cases.h
.PHONY: test-lm-throttle sanitize-lm-throttle c64-lm-throttle vice-lm-throttle lm-throttle-differential
all: build/test-lm-throttle
test: test-lm-throttle
sanitize: sanitize-lm-throttle
build/test-lm-throttle: $(LM_THR_SRC) tests/test_lm_throttle.c tests/lm_throttle_vectors.h $(LM_THR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DLM_THR_COUNT_PATHS $(LM_THR_SRC) tests/test_lm_throttle.c -o $@
test-lm-throttle: build/test-lm-throttle
	./build/test-lm-throttle
	python3 tools/lm_throttle_source.py
sanitize-lm-throttle: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -DLM_THR_COUNT_PATHS -fsanitize=address,undefined $(LM_THR_SRC) tests/test_lm_throttle.c -o build/test-lm-throttle-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-throttle-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-throttle-native: $(LM_THR_SRC) platform/host/lm_throttle_snapshot.c $(LM_THR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_THR_SRC) platform/host/lm_throttle_snapshot.c -o $@
lm-throttle-differential: build/lm-throttle-native
	python3 tools/lm_throttle_freeze.py
	git diff --exit-code -- tests/lm_throttle_vectors.h tests/lm_throttle_c64_vectors.h
c64-lm-throttle: build/lm-throttle.prg
build/lm-throttle.prg: $(LM_THR_SRC) platform/c64/lm_throttle_diagnostic.c tests/lm_throttle_c64_vectors.h $(LM_THR_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-throttle.map $(LM_THR_SRC) platform/c64/lm_throttle_diagnostic.c -o $@
vice-lm-throttle: build/lm-throttle.prg
	python3 tools/vice_run.py build/lm-throttle.prg --cycles 3000000000 --timeout 1200
