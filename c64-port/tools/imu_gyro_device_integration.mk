# SPDX-License-Identifier: MIT
all: build/imu-gyro-device-integrated-stream
build/imu-gyro-device-integrated-stream: tools/imu_gyro_device_snapshot.c src/imu_gyro_device.c src/agc_word.c src/agc_multiword.c tests/imu_gyro_device_cases.c tests/imu_pulse_cases.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-imu-gyro-device-integrated
test-imu-gyro-device-integrated:
	python3 tools/imu_gyro_device_port_host.py --cc "$(CC)"
sanitize: sanitize-imu-gyro-device-integrated
sanitize-imu-gyro-device-integrated:
	python3 tools/imu_gyro_device_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-imu-gyro-device-integrated sanitize-imu-gyro-device-integrated
