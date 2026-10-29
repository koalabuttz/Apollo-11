# SPDX-License-Identifier: MIT
IMU_PULSE_INTEGRATED_CORE = src/imu_pulse.c src/imu_pulse_service.c src/lm_waitlist_image.c src/waitlist.c src/cm_waitlist_image.c src/cm_waitlist.c src/cm_native_image.c src/agc_multiword.c src/agc_word.c tests/imu_pulse_cases.c
all: build/imu-pulse-integrated-stream
build/imu-pulse-integrated-stream: tools/imu_pulse_snapshot.c $(IMU_PULSE_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
test: test-imu-pulse-integrated
test-imu-pulse-integrated: build/imu-pulse-integrated-stream
	python3 tools/imu_pulse_port_host.py --cc "$(CC)"
sanitize: sanitize-imu-pulse-integrated
sanitize-imu-pulse-integrated: | build
	python3 tools/imu_pulse_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-imu-pulse-integrated sanitize-imu-pulse-integrated
