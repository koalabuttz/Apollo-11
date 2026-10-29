# SPDX-License-Identifier: MIT
all: build/imu-standby-integrated-stream
build/imu-standby-integrated-stream: tools/imu_standby_snapshot.c src/imu_standby.c src/imu_mode_control.c src/imu_coarse.c src/agc_word.c src/agc_multiword.c tests/imu_standby_cases.c tests/imu_mode_control_cases.c tests/imu_coarse_cases.c tests/imu_pulse_cases.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-imu-standby-integrated
test-imu-standby-integrated:
	python3 tools/imu_standby_port_host.py --cc "$(CC)"
sanitize: sanitize-imu-standby-integrated
sanitize-imu-standby-integrated:
	python3 tools/imu_standby_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-imu-standby-integrated sanitize-imu-standby-integrated
