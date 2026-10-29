# SPDX-License-Identifier: MIT
all: build/imu-mode-transfer-integrated-stream
build/imu-mode-transfer-integrated-stream: tools/imu_mode_transfer_snapshot.c src/imu_mode_transfer.c src/imu_mode_control.c src/imu_coarse.c src/agc_word.c src/agc_multiword.c tests/imu_mode_transfer_cases.c tests/imu_mode_control_cases.c tests/imu_coarse_cases.c tests/imu_pulse_cases.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-imu-mode-transfer-integrated
test-imu-mode-transfer-integrated:
	python3 tools/imu_mode_transfer_port_host.py --cc "$(CC)"
sanitize: sanitize-imu-mode-transfer-integrated
sanitize-imu-mode-transfer-integrated:
	python3 tools/imu_mode_transfer_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-imu-mode-transfer-integrated sanitize-imu-mode-transfer-integrated
