# SPDX-License-Identifier: MIT
# Milestone lm-dapidler (lane F): Luminary099 DAPIDLER_PROGRAM as native
# routines of the control-transfer ABI.
LM_DI_SRC = src/lm_dapidler.c src/agc_image_ops.c src/agc_hardware_divide.c src/lm_interbank_routines.c src/lm_interbank.c src/lm_exec_routines.c src/lm_admission_image.c src/lm_waitlist_image.c src/waitlist.c src/executive.c src/p66_native.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_dapidler_cases.c
LM_DI_HDR = include/lm_dapidler.h include/lm_flight_io.h include/agc_image_ops.h include/agc_transfer.h tests/lm_dapidler_cases.h
LM_DI_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-dapidler sanitize-lm-dapidler c64-lm-dapidler vice-lm-dapidler lm-dapidler-differential
all: build/test-lm-dapidler
test: test-lm-dapidler
sanitize: sanitize-lm-dapidler
build/test-lm-dapidler: $(LM_DI_SRC) tests/test_lm_dapidler.c tests/lm_dapidler_vectors.h $(LM_DI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_DI_FLAGS) -DLM_DI_COUNT_PATHS $(LM_DI_SRC) tests/test_lm_dapidler.c -o $@
test-lm-dapidler: build/test-lm-dapidler
	./build/test-lm-dapidler
	python3 tools/lm_dapidler_source.py
sanitize-lm-dapidler: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_DI_FLAGS) -DLM_DI_COUNT_PATHS -fsanitize=address,undefined $(LM_DI_SRC) tests/test_lm_dapidler.c -o build/test-lm-dapidler-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-dapidler-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-dapidler-native: $(LM_DI_SRC) platform/host/lm_dapidler_snapshot.c $(LM_DI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_DI_FLAGS) $(LM_DI_SRC) platform/host/lm_dapidler_snapshot.c -o $@
lm-dapidler-differential: build/lm-dapidler-native
	python3 tools/lm_dapidler_freeze.py
	git diff --exit-code -- tests/lm_dapidler_vectors.h tests/lm_dapidler_c64_vectors.h
c64-lm-dapidler: build/lm-dapidler.prg
build/lm-dapidler.prg: $(LM_DI_SRC) platform/c64/lm_dapidler_diagnostic.c tests/lm_dapidler_c64_vectors.h $(LM_DI_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -DP66_NATIVE_DISPATCH_ONLY -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-dapidler.map $(LM_DI_SRC) platform/c64/lm_dapidler_diagnostic.c -o $@
vice-lm-dapidler: build/lm-dapidler.prg
	python3 tools/vice_run.py build/lm-dapidler.prg --cycles 3000000000 --timeout 1200
