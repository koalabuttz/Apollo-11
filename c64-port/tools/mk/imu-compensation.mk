# SPDX-License-Identifier: MIT
# Milestone imu-compensation (lane D1): Luminary099 and Comanche055
# IMU_COMPENSATION_PACKAGE as native routines of the control-transfer ABI,
# composed with INTER-BANK_COMMUNICATION and Luminary NOVAC.
ICP_SRC = src/imu_compensation.c src/agc_image_ops.c src/agc_hardware_divide.c src/lm_interbank_routines.c src/lm_interbank.c src/cm_interbank_routines.c src/cm_interbank_complete.c src/lm_exec_routines.c src/lm_admission_image.c src/lm_waitlist_image.c src/waitlist.c src/executive.c src/p66_native.c src/cm_exec_routines.c src/cm_admission_zero.c src/cm_waitlist_extended.c src/cm_waitlist_internal.c src/cm_task_transfer.c src/cm_native_image.c src/cm_executive.c src/cm_waitlist.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/imu_compensation_cases.c
ICP_HDR = include/imu_compensation.h include/agc_image_ops.h include/agc_transfer.h src/lm_interbank_sites.inc src/cm_interbank_sites.inc src/lm_exec_sites.inc src/cm_exec_sites.inc tests/imu_compensation_cases.h
ICP_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-imu-compensation sanitize-imu-compensation c64-imu-compensation vice-imu-compensation imu-compensation-differential
all: build/test-imu-compensation
test: test-imu-compensation
sanitize: sanitize-imu-compensation
build/test-imu-compensation: $(ICP_SRC) tests/test_imu_compensation.c tests/imu_compensation_vectors.h $(ICP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ICP_FLAGS) $(ICP_SRC) tests/test_imu_compensation.c -o $@
test-imu-compensation: build/test-imu-compensation
	./build/test-imu-compensation
sanitize-imu-compensation: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(ICP_FLAGS) -fsanitize=address,undefined $(ICP_SRC) tests/test_imu_compensation.c -o build/test-imu-compensation-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-imu-compensation-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
build/imu-compensation-native: $(ICP_SRC) platform/host/imu_compensation_snapshot.c $(ICP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ICP_FLAGS) $(ICP_SRC) platform/host/imu_compensation_snapshot.c -o $@
imu-compensation-differential: build/imu-compensation-native
	python3 tools/imu_compensation_source.py
	python3 tools/imu_compensation_freeze.py
	git diff --exit-code -- tests/imu_compensation_vectors.h tests/imu_compensation_c64_vectors.h
ICP_C64_ENTRY_SRC = src/imu_compensation.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/imu_compensation_cases.c
ICP_MOS_FLAGS = -std=c99 -Os -DP66_NATIVE_DISPATCH_ONLY -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
ICP_C64_PRGS = build/imu-compensation.prg build/imu-compensation-lm-trips.prg build/imu-compensation-cm-trips.prg
c64-imu-compensation: $(ICP_C64_PRGS)
build/imu-compensation.prg: $(ICP_C64_ENTRY_SRC) platform/c64/imu_compensation_diagnostic.c tests/imu_compensation_c64_vectors.h $(ICP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(ICP_MOS_FLAGS) -DICP_ENTRY_ONLY -Wl,-Map,build/imu-compensation.map $(ICP_C64_ENTRY_SRC) platform/c64/imu_compensation_diagnostic.c -o $@
build/imu-compensation-lm-trips.prg: $(ICP_C64_ENTRY_SRC) src/lm_interbank_routines.c src/lm_interbank.c platform/c64/imu_compensation_diagnostic.c tests/imu_compensation_c64_vectors.h $(ICP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(ICP_MOS_FLAGS) -DICP_NO_EXEC -DICP_TRIP_LM -Wl,-Map,build/imu-compensation-lm-trips.map $(ICP_C64_ENTRY_SRC) src/lm_interbank_routines.c src/lm_interbank.c platform/c64/imu_compensation_diagnostic.c -o $@
build/imu-compensation-cm-trips.prg: $(ICP_C64_ENTRY_SRC) src/cm_interbank_routines.c src/cm_interbank_complete.c platform/c64/imu_compensation_diagnostic.c tests/imu_compensation_c64_vectors.h $(ICP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(ICP_MOS_FLAGS) -DICP_NO_EXEC -DICP_TRIP_CM -Wl,-Map,build/imu-compensation-cm-trips.map $(ICP_C64_ENTRY_SRC) src/cm_interbank_routines.c src/cm_interbank_complete.c platform/c64/imu_compensation_diagnostic.c -o $@
vice-imu-compensation: $(ICP_C64_PRGS)
	for prg in $(ICP_C64_PRGS); do echo "PRG $$(basename $$prg) sha256 $$(sha256sum $$prg | cut -c1-64)"; python3 tools/vice_run.py $$prg --cycles 3000000000 --timeout 1200 || exit 1; done
