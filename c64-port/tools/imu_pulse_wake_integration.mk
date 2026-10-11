# SPDX-License-Identifier: MIT
all: build/imu-pulse-wake-integrated-stream
build/imu-pulse-wake-integrated-stream: tools/imu_pulse_wake_snapshot.c src/imu_pulse_wake.c src/imu_pulse_service.c src/lm_basic_sleep.c src/executive.c src/cm_basic_sleep.c src/cm_executive.c src/cm_dispatch.c src/lm_waitlist_image.c src/waitlist.c src/cm_waitlist_image.c src/cm_waitlist.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c tests/imu_pulse_wake_cases.c tests/imu_pulse_cases.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-imu-pulse-wake-integrated
test-imu-pulse-wake-integrated:
	python3 tools/imu_pulse_wake_port_host.py --cc "$(CC)"
sanitize: sanitize-imu-pulse-wake-integrated
sanitize-imu-pulse-wake-integrated:
	python3 tools/imu_pulse_wake_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-imu-pulse-wake-integrated sanitize-imu-pulse-wake-integrated
