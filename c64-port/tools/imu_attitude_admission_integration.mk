# SPDX-License-Identifier: MIT
all: build/imu-attitude-admission-integrated-stream
build/imu-attitude-admission-integrated-stream: tools/imu_attitude_admission_snapshot.c src/imu_attitude_admission.c src/imu_mode_control.c src/imu_coarse.c src/agc_word.c src/agc_multiword.c tests/imu_attitude_admission_cases.c tests/imu_mode_control_cases.c tests/imu_coarse_cases.c tests/imu_pulse_cases.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-imu-attitude-admission-integrated
test-imu-attitude-admission-integrated:
	python3 tools/imu_attitude_admission_port_host.py --cc "$(CC)"
sanitize: sanitize-imu-attitude-admission-integrated
sanitize-imu-attitude-admission-integrated:
	python3 tools/imu_attitude_admission_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-imu-attitude-admission-integrated sanitize-imu-attitude-admission-integrated
