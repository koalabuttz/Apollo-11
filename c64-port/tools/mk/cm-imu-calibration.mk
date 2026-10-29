# SPDX-License-Identifier: MIT
# Milestone cm-imu-calibration (lane D1): Comanche055 IMU_CALIBRATION_AND_ALIGNMENT
# compiled by tools/interp_compile.py into native routines of the control-transfer
# ABI, with the input channels it reads latched by the caller.
CIC_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/agc_fast_ops.c src/agc_fast_window.c tests/interp_case.c
CIC_SRC = src/cm_imu_calibration.c tests/cm_imu_calibration_target.c $(CIC_COMMON)
CIC_HDR = include/cm_imu_calibration.h include/agc_input_channels.h tests/interp_case.h tests/cm_imu_calibration_vectors.h tests/cm_imu_calibration_c64_vectors.h
CIC_MOS_FLAGS = -std=c99 -Oz -fno-inline -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -DIRC_INPUTS
.PHONY: test-cm-imu-calibration sanitize-cm-imu-calibration c64-cm-imu-calibration vice-cm-imu-calibration cm-imu-calibration-differential
all: build/test-cm-imu-calibration
test: test-cm-imu-calibration
sanitize: sanitize-cm-imu-calibration
build/test-cm-imu-calibration: $(CIC_SRC) tests/test_cm_imu_calibration.c $(CIC_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CIC_SRC) tests/test_cm_imu_calibration.c -o $@
test-cm-imu-calibration: build/test-cm-imu-calibration
	./build/test-cm-imu-calibration
sanitize-cm-imu-calibration: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CIC_SRC) tests/test_cm_imu_calibration.c -o build/test-cm-imu-calibration-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-imu-calibration-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
cm-imu-calibration-differential:
	python3 tools/interp_routine_differential.py cm-imu-calibration --cases 512 --freeze
	git diff --exit-code -- tests/cm_imu_calibration_vectors.h tests/cm_imu_calibration_c64_vectors.h tools/mk/cm-imu-calibration.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/cm-imu-calibration.groups
CIC_C64_PRGS = $(foreach g,$(CM_IMU_CALIBRATION_GROUPS),build/cm-imu-calibration-$(g).prg)
CIC_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c src/agc_fast_ops.c src/agc_fast_window.c tests/interp_case.c tests/interp_cut.c tests/cm_imu_calibration_target.c
CIC_C64_cm = src/cm_imu_calibration.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-cm-imu-calibration: $(CIC_C64_PRGS)
build/cm-imu-calibration-%.prg: $(CIC_SRC) platform/c64/cm_imu_calibration_diagnostic.c tools/mk/cm-imu-calibration.groups $(CIC_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CIC_MOS_FLAGS) $(CM_IMU_CALIBRATION_GROUP_FLAGS_$*) -Wl,-Map,build/cm-imu-calibration-$*.map $(CIC_C64_$(CM_IMU_CALIBRATION_GROUP_MISSION_$*)) $(CIC_C64_COMMON) platform/c64/cm_imu_calibration_diagnostic.c -o $@
vice-cm-imu-calibration: $(CIC_C64_PRGS)
	for p in $(CIC_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
