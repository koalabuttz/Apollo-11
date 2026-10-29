# SPDX-License-Identifier: MIT
all: build/imu-mode-control-integrated-stream
build/imu-mode-control-integrated-stream: tools/imu_mode_control_snapshot.c src/imu_mode_control.c src/imu_coarse.c src/agc_word.c src/agc_multiword.c tests/imu_mode_control_cases.c tests/imu_coarse_cases.c tests/imu_pulse_cases.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-imu-mode-control-integrated
test-imu-mode-control-integrated:
	python3 tools/imu_mode_control_port_host.py --cc "$(CC)"
sanitize: sanitize-imu-mode-control-integrated
sanitize-imu-mode-control-integrated:
	python3 tools/imu_mode_control_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-imu-mode-control-integrated sanitize-imu-mode-control-integrated
