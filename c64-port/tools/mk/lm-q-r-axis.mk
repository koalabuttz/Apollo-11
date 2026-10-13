# SPDX-License-Identifier: MIT
# Milestone lm-q-r-axis (lane F): Luminary099 Q_R-AXIS_RCS_AUTOPILOT as native
# routines of the control-transfer ABI.
LM_QR_SRC = src/lm_q_r_axis.c src/agc_fast_ops.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_q_r_axis_cases.c
LM_QR_HDR = include/lm_q_r_axis.h include/lm_flight_io.h include/agc_image_ops.h include/agc_fast_ops.h include/agc_image_thin.h include/agc_fast_redirect.h include/agc_transfer.h src/lm_q_r_axis_sites.inc tests/lm_q_r_axis_cases.h
LM_QR_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-q-r-axis sanitize-lm-q-r-axis c64-lm-q-r-axis vice-lm-q-r-axis lm-q-r-axis-differential
all: build/test-lm-q-r-axis
test: test-lm-q-r-axis
sanitize: sanitize-lm-q-r-axis
build/test-lm-q-r-axis: $(LM_QR_SRC) src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c tests/test_lm_q_r_axis.c tests/lm_q_r_axis_vectors.h $(LM_QR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_QR_FLAGS) -DLM_QR_COUNT_PATHS $(LM_QR_SRC) src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c tests/test_lm_q_r_axis.c -o $@
test-lm-q-r-axis: build/test-lm-q-r-axis
	./build/test-lm-q-r-axis
	python3 tools/lm_q_r_axis_source.py
sanitize-lm-q-r-axis: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_QR_FLAGS) -DLM_QR_COUNT_PATHS -fsanitize=address,undefined $(LM_QR_SRC) src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c tests/test_lm_q_r_axis.c -o build/test-lm-q-r-axis-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-q-r-axis-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-q-r-axis-native: $(LM_QR_SRC) platform/host/lm_q_r_axis_snapshot.c $(LM_QR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_QR_FLAGS) $(LM_QR_SRC) platform/host/lm_q_r_axis_snapshot.c -o $@
lm-q-r-axis-differential: build/lm-q-r-axis-native
	python3 tools/lm_q_r_axis_freeze.py
	git diff --exit-code -- tests/lm_q_r_axis_vectors.h tests/lm_q_r_axis_c64_vectors.h
c64-lm-q-r-axis: build/lm-q-r-axis.prg
build/lm-q-r-axis.prg: $(LM_QR_SRC) platform/c64/lm_q_r_axis_diagnostic.c tests/lm_q_r_axis_c64_vectors.h $(LM_QR_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -DP66_NATIVE_DISPATCH_ONLY -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-q-r-axis.map $(LM_QR_SRC) platform/c64/lm_q_r_axis_diagnostic.c -o $@
vice-lm-q-r-axis: build/lm-q-r-axis.prg
	python3 tools/vice_run.py build/lm-q-r-axis.prg --cycles 3000000000 --timeout 1200
