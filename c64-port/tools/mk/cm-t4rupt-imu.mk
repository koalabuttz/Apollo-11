# SPDX-License-Identifier: MIT
# Milestone cm-t4rupt-imu (lane A): the IMU monitor of Comanche055
# T4RUPT_PROGRAM.agc (IMUMON to 90SECS, with ZOPFIN3) as cm_t4rupt_imu_routines.
CTI_SRC = src/cm_t4rupt_imu.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_t4rupt_imu_cases.c
CTI_HDR = include/cm_t4rupt_imu.h include/agc_transfer.h include/agc_image_ops.h tests/cm_t4rupt_imu_cases.h
CTI_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-cm-t4rupt-imu sanitize-cm-t4rupt-imu c64-cm-t4rupt-imu vice-cm-t4rupt-imu cm-t4rupt-imu-differential
all: build/test-cm-t4rupt-imu
test: test-cm-t4rupt-imu
sanitize: sanitize-cm-t4rupt-imu
build/test-cm-t4rupt-imu: $(CTI_SRC) tests/test_cm_t4rupt_imu.c tests/cm_t4rupt_imu_vectors.h $(CTI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CTI_FLAGS) $(CTI_SRC) tests/test_cm_t4rupt_imu.c -o $@
test-cm-t4rupt-imu: build/test-cm-t4rupt-imu
	./build/test-cm-t4rupt-imu
sanitize-cm-t4rupt-imu: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(CTI_FLAGS) -fsanitize=address,undefined $(CTI_SRC) tests/test_cm_t4rupt_imu.c -o build/test-cm-t4rupt-imu-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-t4rupt-imu-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-t4rupt-imu-native: $(CTI_SRC) platform/host/cm_t4rupt_imu_snapshot.c $(CTI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CTI_FLAGS) $(CTI_SRC) platform/host/cm_t4rupt_imu_snapshot.c -o $@
cm-t4rupt-imu-differential: build/cm-t4rupt-imu-native
	python3 tools/cm_t4rupt_imu_freeze.py
	python3 tools/cm_t4rupt_imu_source_map.py
	git diff --exit-code -- tests/cm_t4rupt_imu_vectors.h tests/cm_t4rupt_imu_c64_vectors.h
c64-cm-t4rupt-imu: build/cm-t4rupt-imu.prg
build/cm-t4rupt-imu.prg: $(CTI_SRC) platform/c64/cm_t4rupt_imu_diagnostic.c tests/cm_t4rupt_imu_c64_vectors.h $(CTI_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(CTI_FLAGS) -Wl,-Map,build/cm-t4rupt-imu.map $(CTI_SRC) platform/c64/cm_t4rupt_imu_diagnostic.c -o $@
vice-cm-t4rupt-imu: build/cm-t4rupt-imu.prg
	python3 tools/vice_run.py build/cm-t4rupt-imu.prg --cycles 3000000000 --timeout 1200
