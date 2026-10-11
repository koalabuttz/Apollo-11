# SPDX-License-Identifier: MIT
IMU_GYRO_INTEGRATED_CORE = src/imu_pulse_task.c src/imu_pulse_task_register.c src/lm_waitlist_image.c src/waitlist.c src/cm_waitlist_image.c src/cm_waitlist.c src/cm_native_image.c src/agc_multiword.c src/agc_word.c tests/imu_pulse_cases.c tests/imu_pulse_task_cases.c
all: build/imu-pulse-task-integrated-stream
build/imu-pulse-task-integrated-stream: tools/imu_pulse_task_snapshot.c $(IMU_GYRO_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
test: test-imu-pulse-task-integrated
test-imu-pulse-task-integrated: build/imu-pulse-task-integrated-stream
	python3 tools/imu_pulse_task_port_host.py --cc "$(CC)"
sanitize: sanitize-imu-pulse-task-integrated
sanitize-imu-pulse-task-integrated: | build
	python3 tools/imu_pulse_task_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-imu-pulse-task-integrated sanitize-imu-pulse-task-integrated
