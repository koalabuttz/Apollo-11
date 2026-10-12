# SPDX-License-Identifier: MIT
# Milestone lm-imu-performance (lane D1): Luminary099 IMU_PERFORMANCE_TEST_2
# and IMU_PERFORMANCE_TESTS_4 (P07) as native routines of the
# control-transfer ABI.
IPT_SRC = src/lm_imu_performance.c src/sband_trig.c src/guidance_trig.c src/guidance_product.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_imu_performance_cases.c
IPT_HDR = include/lm_imu_performance.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h include/agc_transfer.h include/sband_trig.h tests/lm_imu_performance_cases.h
IPT_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-imu-performance sanitize-lm-imu-performance c64-lm-imu-performance vice-lm-imu-performance lm-imu-performance-differential
all: build/test-lm-imu-performance
test: test-lm-imu-performance
sanitize: sanitize-lm-imu-performance
build/test-lm-imu-performance: $(IPT_SRC) tests/test_lm_imu_performance.c tests/lm_imu_performance_vectors.h $(IPT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(IPT_FLAGS) $(IPT_SRC) tests/test_lm_imu_performance.c -o $@
# The unit test again with the C64 parts' jump dispatch (part 0: everything).
build/test-lm-imu-performance-part0: $(IPT_SRC) tests/test_lm_imu_performance.c tests/lm_imu_performance_vectors.h $(IPT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(IPT_FLAGS) -DIPT_C64_PART=0 $(IPT_SRC) tests/test_lm_imu_performance.c -o $@
test-lm-imu-performance: build/test-lm-imu-performance build/test-lm-imu-performance-part0
	./build/test-lm-imu-performance
	./build/test-lm-imu-performance-part0
sanitize-lm-imu-performance: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic $(IPT_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer $(IPT_SRC) tests/test_lm_imu_performance.c -o build/test-lm-imu-performance-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-imu-performance-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary listing build).
build/lm-imu-performance-native: $(IPT_SRC) platform/host/lm_imu_performance_snapshot.c $(IPT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(IPT_FLAGS) $(IPT_SRC) platform/host/lm_imu_performance_snapshot.c -o $@
lm-imu-performance-differential: build/lm-imu-performance-native
	python3 tools/lm_imu_performance_source.py
	python3 tools/prepare_no_imu_device_engine.py
	python3 tools/lm_imu_performance_freeze.py
	git diff --exit-code -- tests/lm_imu_performance_vectors.h tests/lm_imu_performance_c64_vectors.h
# C64: seven parts (IPT_C64_PART, tests/lm_imu_performance_cases.h), each
# linking the code its cases run; images under KERNAL ROM.
IPT_MOS_FLAGS = -std=c99 -Oz -DAPOLLO_FIXED_LM_ONLY -DP66_NATIVE_DISPATCH_ONLY -DIPT_UNDO_ADDRESS=0xf000u -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
IPT_C64_PRGS = $(foreach p,1 2 3 4 5 6 7,build/lm-imu-performance-$(p).prg)
c64-lm-imu-performance: $(IPT_C64_PRGS)
build/lm-imu-performance-%.prg: $(IPT_SRC) platform/c64/lm_imu_performance_diagnostic.c tests/lm_imu_performance_c64_vectors.h $(IPT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(IPT_MOS_FLAGS) -DIPT_C64_PART=$* -Wl,-Map,build/lm-imu-performance-$*.map $(IPT_SRC) platform/c64/lm_imu_performance_diagnostic.c -o $@
vice-lm-imu-performance: $(IPT_C64_PRGS)
	for prg in $(IPT_C64_PRGS); do python3 tools/vice_run.py $$prg --cycles 3000000000 --timeout 1500 || exit 1; done
