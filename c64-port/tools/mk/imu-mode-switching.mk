# SPDX-License-Identifier: MIT
# Milestone imu-mode-switching (lane D1): Luminary099 and Comanche055
# IMU_MODE_SWITCHING_ROUTINES as native routines of the control-transfer
# ABI, composed with INTER-BANK_COMMUNICATION.
IMS_SRC = src/imu_mode_switching.c src/agc_image_ops.c src/lm_interbank_routines.c src/lm_interbank.c src/cm_interbank_routines.c src/cm_interbank_complete.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/agc_hardware_divide.c tests/imu_mode_switching_cases.c
IMS_HDR = include/imu_mode_switching.h include/agc_image_ops.h include/agc_transfer.h src/lm_interbank_sites.inc src/cm_interbank_sites.inc tests/imu_mode_switching_cases.h
IMS_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-imu-mode-switching sanitize-imu-mode-switching c64-imu-mode-switching vice-imu-mode-switching imu-mode-switching-differential
all: build/test-imu-mode-switching
test: test-imu-mode-switching
sanitize: sanitize-imu-mode-switching
build/test-imu-mode-switching: $(IMS_SRC) tests/test_imu_mode_switching.c tests/imu_mode_switching_vectors.h $(IMS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(IMS_FLAGS) $(IMS_SRC) tests/test_imu_mode_switching.c -o $@
test-imu-mode-switching: build/test-imu-mode-switching
	./build/test-imu-mode-switching
sanitize-imu-mode-switching: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(IMS_FLAGS) -fsanitize=address,undefined $(IMS_SRC) tests/test_imu_mode_switching.c -o build/test-imu-mode-switching-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-imu-mode-switching-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
build/imu-mode-switching-native: $(IMS_SRC) platform/host/imu_mode_switching_snapshot.c $(IMS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(IMS_FLAGS) $(IMS_SRC) platform/host/imu_mode_switching_snapshot.c -o $@
imu-mode-switching-differential: build/imu-mode-switching-native
	python3 tools/imu_mode_switching_source.py
	python3 tools/prepare_no_imu_device_engine.py
	python3 tools/imu_mode_switching_freeze.py
	python3 tools/imu_mode_switching_cover.py
	git diff --exit-code -- tests/imu_mode_switching_vectors.h tests/imu_mode_switching_c64_vectors.h
# C64: each third of the routines (IMS_C64_PART) as its own entry, Luminary
# round-trip and Comanche round-trip program; the other routines are
# compiled out, hence -Wno-unused-function.
IMS_C64_ENTRY_SRC = src/imu_mode_switching.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/imu_mode_switching_cases.c
IMS_MOS_FLAGS = -std=c99 -Oz -DP66_NATIVE_DISPATCH_ONLY -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wno-unused-function
IMS_C64_DEPS = $(IMS_C64_ENTRY_SRC) platform/c64/imu_mode_switching_diagnostic.c tests/imu_mode_switching_c64_vectors.h $(IMS_HDR)
IMS_C64_PRGS = $(foreach p,1 2 3,build/imu-mode-switching-$(p).prg build/imu-mode-switching-lm-trips-$(p).prg build/imu-mode-switching-cm-trips-$(p).prg)
c64-imu-mode-switching: $(IMS_C64_PRGS)
build/imu-mode-switching-%.prg: $(IMS_C64_DEPS) | build
	$(MOS_CC) $(CPPFLAGS) $(IMS_MOS_FLAGS) -DIMS_C64_PART=$* -DIMS_ENTRY_ONLY -Wl,-Map,build/imu-mode-switching-$*.map $(IMS_C64_ENTRY_SRC) platform/c64/imu_mode_switching_diagnostic.c -o $@
build/imu-mode-switching-lm-trips-%.prg: $(IMS_C64_DEPS) src/lm_interbank_routines.c src/lm_interbank.c | build
	$(MOS_CC) $(CPPFLAGS) $(IMS_MOS_FLAGS) -DIMS_C64_PART=$* -DIMS_TRIP_LM -Wl,-Map,build/imu-mode-switching-lm-trips-$*.map $(IMS_C64_ENTRY_SRC) src/lm_interbank_routines.c src/lm_interbank.c platform/c64/imu_mode_switching_diagnostic.c -o $@
build/imu-mode-switching-cm-trips-%.prg: $(IMS_C64_DEPS) src/cm_interbank_routines.c src/cm_interbank_complete.c | build
	$(MOS_CC) $(CPPFLAGS) $(IMS_MOS_FLAGS) -DIMS_C64_PART=$* -DIMS_TRIP_CM -Wl,-Map,build/imu-mode-switching-cm-trips-$*.map $(IMS_C64_ENTRY_SRC) src/cm_interbank_routines.c src/cm_interbank_complete.c platform/c64/imu_mode_switching_diagnostic.c -o $@
vice-imu-mode-switching: $(IMS_C64_PRGS)
	for prg in $(IMS_C64_PRGS); do echo "PRG $$(basename $$prg) sha256 $$(sha256sum $$prg | cut -c1-64)"; python3 tools/vice_run.py $$prg --cycles 3000000000 --timeout 1200 || exit 1; done
