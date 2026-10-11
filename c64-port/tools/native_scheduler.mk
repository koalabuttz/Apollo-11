# SPDX-License-Identifier: MIT
NSCHED_FLAGS = $(DJOB_FLAGS) -DEXEC_NATIVE_RESIDENT_SERVICES
NSCHED_CORE = $(DJOB_CORE) src/native_overlay_dispatch.c
build/display-job-scheduler-native: tools/display_job_scheduler_snapshot.c $(NSCHED_CORE) | build
	$(CC) -O2 $(NSCHED_FLAGS) $^ -o $@
build/display-job-lifecycle-scheduler-native: tools/display_job_scheduler_snapshot.c tests/display_job_lifecycle_cases.c $(NSCHED_CORE) | build
	$(CC) -O2 -DDISPLAY_JOB_LIFECYCLE $(NSCHED_FLAGS) $^ -o $@
native_scheduler-host: build/display-job-scheduler-native build/display-job-lifecycle-scheduler-native
	build/display-job-scheduler-native > build/display-job-scheduler.snapshots
	cmp build/display-job-scheduler.snapshots build/display-job-reference.snapshots
	build/display-job-lifecycle-scheduler-native > build/display-job-lifecycle-scheduler.snapshots
	cmp build/display-job-lifecycle-scheduler.snapshots build/display-job-lifecycle-reference.snapshots
native_scheduler-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(NSCHED_FLAGS) tools/display_job_scheduler_snapshot.c $(NSCHED_CORE) -o build/display-job-scheduler-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/display-job-scheduler-sanitize > build/display-job-scheduler-sanitize.snapshots
	cmp build/display-job-scheduler-sanitize.snapshots build/display-job-reference.snapshots
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -DDISPLAY_JOB_LIFECYCLE $(NSCHED_FLAGS) tools/display_job_scheduler_snapshot.c tests/display_job_lifecycle_cases.c $(NSCHED_CORE) -o build/display-job-lifecycle-scheduler-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/display-job-lifecycle-scheduler-sanitize > build/display-job-lifecycle-scheduler-sanitize.snapshots
	cmp build/display-job-lifecycle-scheduler-sanitize.snapshots build/display-job-lifecycle-reference.snapshots
native_scheduler-c64:
	python3 tools/native_scheduler_overlay_build.py --cc $(MOS_CC)
.PHONY: native_scheduler-host native_scheduler-sanitize native_scheduler-c64 native_scheduler-guards
native_scheduler-guards:
	$(CC) -O2 -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -DEXEC_NATIVE_RESIDENT_SERVICES tests/test_native_scheduler.c src/native_overlay_dispatch.c src/native_dispatch.c src/irq_delivery.c src/executive.c src/agc_word.c -o build/test-native-scheduler
	build/test-native-scheduler
	$(CC) -O1 -g -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -DEXEC_NATIVE_RESIDENT_SERVICES -fsanitize=address,undefined -fno-omit-frame-pointer tests/test_native_scheduler.c src/native_overlay_dispatch.c src/native_dispatch.c src/irq_delivery.c src/executive.c src/agc_word.c -o build/test-native-scheduler-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-native-scheduler-sanitize

native_scheduler-final:
	$(CC) -O2 -DDISPLAY_JOB_FINAL $(NSCHED_FLAGS) tools/display_job_scheduler_snapshot.c tests/display_job_final_cases.c $(NSCHED_CORE) -o build/display-job-final-scheduler-native
	build/display-job-final-scheduler-native > build/display-job-final-scheduler.snapshots
	cmp build/display-job-final-scheduler.snapshots build/display-job-final-reference.snapshots
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -DDISPLAY_JOB_FINAL $(NSCHED_FLAGS) tools/display_job_scheduler_snapshot.c tests/display_job_final_cases.c $(NSCHED_CORE) -o build/display-job-final-scheduler-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/display-job-final-scheduler-sanitize > build/display-job-final-scheduler-sanitize.snapshots
	cmp build/display-job-final-scheduler-sanitize.snapshots build/display-job-final-reference.snapshots
