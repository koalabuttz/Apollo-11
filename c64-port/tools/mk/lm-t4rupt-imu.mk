# SPDX-License-Identifier: MIT
# Milestone lm-t4rupt-imu (lane A): the IMU monitor of Luminary099
# T4RUPT_PROGRAM.agc (IMUMON to 90SECS) as lm_t4rupt_imu_routines.
LTI_SRC = src/lm_t4rupt_imu.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_t4rupt_imu_cases.c
LTI_HDR = include/lm_t4rupt_imu.h include/agc_transfer.h include/agc_image_ops.h tests/lm_t4rupt_imu_cases.h
LTI_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-t4rupt-imu sanitize-lm-t4rupt-imu c64-lm-t4rupt-imu vice-lm-t4rupt-imu lm-t4rupt-imu-differential
all: build/test-lm-t4rupt-imu
test: test-lm-t4rupt-imu
sanitize: sanitize-lm-t4rupt-imu
build/test-lm-t4rupt-imu: $(LTI_SRC) tests/test_lm_t4rupt_imu.c tests/lm_t4rupt_imu_vectors.h $(LTI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LTI_FLAGS) $(LTI_SRC) tests/test_lm_t4rupt_imu.c -o $@
test-lm-t4rupt-imu: build/test-lm-t4rupt-imu
	./build/test-lm-t4rupt-imu
sanitize-lm-t4rupt-imu: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LTI_FLAGS) -fsanitize=address,undefined $(LTI_SRC) tests/test_lm_t4rupt_imu.c -o build/test-lm-t4rupt-imu-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-t4rupt-imu-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-t4rupt-imu-native: $(LTI_SRC) platform/host/lm_t4rupt_imu_snapshot.c $(LTI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LTI_FLAGS) $(LTI_SRC) platform/host/lm_t4rupt_imu_snapshot.c -o $@
lm-t4rupt-imu-differential: build/lm-t4rupt-imu-native
	python3 tools/lm_t4rupt_imu_freeze.py
	python3 tools/lm_t4rupt_imu_source_map.py
	git diff --exit-code -- tests/lm_t4rupt_imu_vectors.h tests/lm_t4rupt_imu_c64_vectors.h
c64-lm-t4rupt-imu: build/lm-t4rupt-imu.prg
build/lm-t4rupt-imu.prg: $(LTI_SRC) platform/c64/lm_t4rupt_imu_diagnostic.c tests/lm_t4rupt_imu_c64_vectors.h $(LTI_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(LTI_FLAGS) -Wl,-Map,build/lm-t4rupt-imu.map $(LTI_SRC) platform/c64/lm_t4rupt_imu_diagnostic.c -o $@
vice-lm-t4rupt-imu: build/lm-t4rupt-imu.prg
	python3 tools/vice_run.py build/lm-t4rupt-imu.prg --cycles 3000000000 --timeout 1200
