# SPDX-License-Identifier: MIT
IMU_END_INTEGRATED_CORE = src/imu_pulse_end.c src/lm_basic_wake.c src/executive.c src/cm_basic_wake.c src/cm_executive.c src/cm_dispatch.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c tests/imu_pulse_end_cases.c tests/imu_pulse_cases.c
all: build/imu-pulse-end-integrated-stream
build/imu-pulse-end-integrated-stream: tools/imu_pulse_end_snapshot.c $(IMU_END_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
test: test-imu-pulse-end-integrated
test-imu-pulse-end-integrated: build/imu-pulse-end-integrated-stream
	python3 tools/imu_pulse_end_port_host.py --cc "$(CC)"
sanitize: sanitize-imu-pulse-end-integrated
sanitize-imu-pulse-end-integrated: | build
	python3 tools/imu_pulse_end_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-imu-pulse-end-integrated sanitize-imu-pulse-end-integrated
