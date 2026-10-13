# SPDX-License-Identifier: MIT
# Milestone lm-p-axis (lane F): Luminary099 P-AXIS_RCS_AUTOPILOT as native
# routines of the control-transfer ABI.
LM_PA_SRC = src/lm_p_axis.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_p_axis_cases.c
LM_PA_HDR = include/lm_p_axis.h include/lm_flight_io.h include/agc_image_ops.h include/agc_transfer.h src/lm_p_axis_sites.inc tests/lm_p_axis_cases.h
LM_PA_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-p-axis sanitize-lm-p-axis c64-lm-p-axis vice-lm-p-axis lm-p-axis-differential
all: build/test-lm-p-axis
test: test-lm-p-axis
sanitize: sanitize-lm-p-axis
build/test-lm-p-axis: $(LM_PA_SRC) src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c tests/test_lm_p_axis.c tests/lm_p_axis_vectors.h $(LM_PA_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_PA_FLAGS) -DLM_PA_COUNT_PATHS $(LM_PA_SRC) src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c tests/test_lm_p_axis.c -o $@
test-lm-p-axis: build/test-lm-p-axis
	./build/test-lm-p-axis
	python3 tools/lm_p_axis_source.py
sanitize-lm-p-axis: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_PA_FLAGS) -DLM_PA_COUNT_PATHS -fsanitize=address,undefined $(LM_PA_SRC) src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c tests/test_lm_p_axis.c -o build/test-lm-p-axis-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p-axis-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-p-axis-native: $(LM_PA_SRC) platform/host/lm_p_axis_snapshot.c $(LM_PA_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_PA_FLAGS) $(LM_PA_SRC) platform/host/lm_p_axis_snapshot.c -o $@
lm-p-axis-differential: build/lm-p-axis-native
	python3 tools/lm_p_axis_freeze.py
	git diff --exit-code -- tests/lm_p_axis_vectors.h tests/lm_p_axis_c64_vectors.h
c64-lm-p-axis: build/lm-p-axis.prg
build/lm-p-axis.prg: $(LM_PA_SRC) platform/c64/lm_p_axis_diagnostic.c tests/lm_p_axis_c64_vectors.h $(LM_PA_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -DP66_NATIVE_DISPATCH_ONLY -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-p-axis.map $(LM_PA_SRC) platform/c64/lm_p_axis_diagnostic.c -o $@
vice-lm-p-axis: build/lm-p-axis.prg
	python3 tools/vice_run.py build/lm-p-axis.prg --cycles 3000000000 --timeout 1200
