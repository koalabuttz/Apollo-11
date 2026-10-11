# SPDX-License-Identifier: MIT
all: build/imu-coarse-integrated-stream
build/imu-coarse-integrated-stream: tools/imu_coarse_snapshot.c src/imu_coarse.c src/agc_word.c src/agc_multiword.c tests/imu_coarse_cases.c tests/imu_pulse_cases.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-imu-coarse-integrated
test-imu-coarse-integrated:
	python3 tools/imu_coarse_port_host.py --cc "$(CC)"
sanitize: sanitize-imu-coarse-integrated
sanitize-imu-coarse-integrated:
	python3 tools/imu_coarse_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-imu-coarse-integrated sanitize-imu-coarse-integrated
