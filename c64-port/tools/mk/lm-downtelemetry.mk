# SPDX-License-Identifier: MIT
# Milestone lm-downtelemetry (lane A2): Luminary099 DOWN-TELEMETRY_PROGRAM
# (DODOWNTM, DNTMGOTO, downlists and erasable dump) as native routines of the
# control-transfer ABI.
LM_DNTM_SRC = src/lm_downtelemetry.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_downtelemetry_cases.c
LM_DNTM_HDR = include/lm_downtelemetry.h include/agc_transfer.h include/agc_image_ops.h src/lm_downtelemetry_data.inc tests/lm_downtelemetry_cases.h
.PHONY: test-lm-downtelemetry sanitize-lm-downtelemetry c64-lm-downtelemetry vice-lm-downtelemetry lm-downtelemetry-differential
all: build/test-lm-downtelemetry
test: test-lm-downtelemetry
sanitize: sanitize-lm-downtelemetry
build/test-lm-downtelemetry: $(LM_DNTM_SRC) tests/test_lm_downtelemetry.c tests/lm_downtelemetry_vectors.h $(LM_DNTM_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_DNTM_SRC) tests/test_lm_downtelemetry.c -o $@
test-lm-downtelemetry: build/test-lm-downtelemetry
	python3 tools/downtelemetry_generate.py --mission Luminary099 --check
	./build/test-lm-downtelemetry
sanitize-lm-downtelemetry: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_DNTM_SRC) tests/test_lm_downtelemetry.c -o build/test-lm-downtelemetry-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-downtelemetry-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-downtelemetry-native: $(LM_DNTM_SRC) platform/host/lm_downtelemetry_snapshot.c $(LM_DNTM_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_DNTM_SRC) platform/host/lm_downtelemetry_snapshot.c -o $@
lm-downtelemetry-differential: build/lm-downtelemetry-native
	python3 tools/lm_downtelemetry_freeze.py
	git diff --exit-code -- tests/lm_downtelemetry_vectors.h tests/lm_downtelemetry_c64_vectors.h
c64-lm-downtelemetry: build/lm-downtelemetry.prg
build/lm-downtelemetry.prg: $(LM_DNTM_SRC) platform/c64/lm_downtelemetry_diagnostic.c tests/lm_downtelemetry_c64_vectors.h $(LM_DNTM_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-downtelemetry.map $(LM_DNTM_SRC) platform/c64/lm_downtelemetry_diagnostic.c -o $@
vice-lm-downtelemetry: build/lm-downtelemetry.prg
	python3 tools/vice_run.py build/lm-downtelemetry.prg --cycles 8000000000 --timeout 2400
