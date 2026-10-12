# SPDX-License-Identifier: MIT
# Milestone lm-trim-gimbal (lane F): Luminary099 TRIM_GIMBAL_CONTROL_SYSTEM
# as native routines of the control-transfer ABI.
LM_TG_SRC = src/lm_trim_gimbal.c src/agc_image_ops.c src/agc_hardware_divide.c src/lm_interbank_routines.c src/lm_interbank.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_trim_gimbal_cases.c
LM_TG_HDR = include/lm_trim_gimbal.h include/lm_flight_io.h include/agc_image_ops.h include/agc_transfer.h tests/lm_trim_gimbal_cases.h
LM_TG_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-trim-gimbal sanitize-lm-trim-gimbal c64-lm-trim-gimbal vice-lm-trim-gimbal lm-trim-gimbal-differential
all: build/test-lm-trim-gimbal
test: test-lm-trim-gimbal
sanitize: sanitize-lm-trim-gimbal
build/test-lm-trim-gimbal: $(LM_TG_SRC) tests/test_lm_trim_gimbal.c tests/lm_trim_gimbal_vectors.h $(LM_TG_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_TG_FLAGS) -DLM_TG_COUNT_PATHS $(LM_TG_SRC) tests/test_lm_trim_gimbal.c -o $@
test-lm-trim-gimbal: build/test-lm-trim-gimbal
	./build/test-lm-trim-gimbal
	python3 tools/lm_trim_gimbal_source.py
sanitize-lm-trim-gimbal: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_TG_FLAGS) -DLM_TG_COUNT_PATHS -fsanitize=address,undefined $(LM_TG_SRC) tests/test_lm_trim_gimbal.c -o build/test-lm-trim-gimbal-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-trim-gimbal-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-trim-gimbal-native: $(LM_TG_SRC) platform/host/lm_trim_gimbal_snapshot.c $(LM_TG_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_TG_FLAGS) $(LM_TG_SRC) platform/host/lm_trim_gimbal_snapshot.c -o $@
lm-trim-gimbal-differential: build/lm-trim-gimbal-native
	python3 tools/lm_trim_gimbal_freeze.py
	git diff --exit-code -- tests/lm_trim_gimbal_vectors.h tests/lm_trim_gimbal_c64_vectors.h
c64-lm-trim-gimbal: build/lm-trim-gimbal.prg
build/lm-trim-gimbal.prg: $(LM_TG_SRC) platform/c64/lm_trim_gimbal_diagnostic.c tests/lm_trim_gimbal_c64_vectors.h $(LM_TG_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -DP66_NATIVE_DISPATCH_ONLY -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-trim-gimbal.map $(LM_TG_SRC) platform/c64/lm_trim_gimbal_diagnostic.c -o $@
vice-lm-trim-gimbal: build/lm-trim-gimbal.prg
	python3 tools/vice_run.py build/lm-trim-gimbal.prg --cycles 3000000000 --timeout 1200
