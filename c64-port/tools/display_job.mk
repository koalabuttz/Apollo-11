# SPDX-License-Identifier: MIT
DJOB_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -Wl,--gc-sections
DJOB_CORE = src/p66_nv_render.c src/dsky_scale.c src/guidance_product.c src/p66_nv_words.c src/p66_nv_rows.c src/p66_nv_entry.c src/dsky_two_digit.c src/dsky_pinball.c src/dsky_decimal.c src/agc_multiword.c src/p66_goplay.c src/p66_makeplay.c src/p66_makeplay_sleep.c src/lm_basic_priority.c src/lm_basic_wake.c src/lm_basic_sleep.c src/lm_basic_finish.c src/lm_pinball_busy.c src/p66_display_job.c src/p66_native.c src/executive.c src/native_dispatch.c src/irq_delivery.c src/agc_word.c tests/throttle_bridge_cases.c tests/makeplay_cases.c tests/display_job_cases.c
build/display-job-native: tools/display_job_snapshot.c $(DJOB_CORE) | build
	$(CC) -O2 $(DJOB_FLAGS) $^ -o $@
build/display-job-lifecycle-native: tools/display_job_lifecycle_snapshot.c tests/display_job_lifecycle_cases.c $(DJOB_CORE) | build
	$(CC) -O2 $(DJOB_FLAGS) $^ -o $@
build/test-display-job: tests/test_display_job.c $(DJOB_CORE) | build
	$(CC) -O2 $(DJOB_FLAGS) $^ -o $@
display_job-host: build/display-job-native build/display-job-lifecycle-native build/test-display-job
	build/test-display-job
	build/display-job-native > build/display-job-strict.snapshots
	cmp build/display-job-strict.snapshots build/display-job-reference.snapshots
	build/display-job-lifecycle-native > build/display-job-lifecycle-strict.snapshots
	cmp build/display-job-lifecycle-strict.snapshots build/display-job-lifecycle-reference.snapshots
display_job-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DJOB_FLAGS) tests/test_display_job.c $(DJOB_CORE) -o build/test-display-job-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-display-job-sanitize
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DJOB_FLAGS) tools/display_job_snapshot.c $(DJOB_CORE) -o build/display-job-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/display-job-native-sanitize > build/display-job-sanitize.snapshots
	cmp build/display-job-sanitize.snapshots build/display-job-reference.snapshots
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DJOB_FLAGS) tools/display_job_lifecycle_snapshot.c tests/display_job_lifecycle_cases.c $(DJOB_CORE) -o build/display-job-lifecycle-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/display-job-lifecycle-native-sanitize > build/display-job-lifecycle-sanitize.snapshots
	cmp build/display-job-lifecycle-sanitize.snapshots build/display-job-lifecycle-reference.snapshots
display_job-differential:
	python3 tools/display_job_differential.py
	python3 tools/display_job_lifecycle_differential.py
all: build/display-job-native build/display-job-lifecycle-native
.PHONY: display_job-host display_job-sanitize display_job-differential
display_job-c64:
	python3 tools/p66_display_job_overlay_build.py --cc $(MOS_CC)

build/display-job-final-native: tools/display_job_final_snapshot.c tests/display_job_final_cases.c $(DJOB_CORE) | build
	$(CC) -O2 $(DJOB_FLAGS) $^ -o $@
display_job-final-host: build/display-job-final-native
	build/display-job-final-native > build/display-job-final-strict.snapshots
	cmp build/display-job-final-strict.snapshots build/display-job-final-reference.snapshots
display_job-final-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DJOB_FLAGS) tools/display_job_final_snapshot.c tests/display_job_final_cases.c $(DJOB_CORE) -o build/display-job-final-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/display-job-final-native-sanitize > build/display-job-final-sanitize.snapshots
	cmp build/display-job-final-sanitize.snapshots build/display-job-final-reference.snapshots

# Source END reached by the still-running descent basic continuation.
display_job-direct-finish:
	$(CC) -O2 -DDISPLAY_JOB_DIRECT_FINISH $(DJOB_FLAGS) tools/display_job_final_snapshot.c tests/display_job_final_cases.c $(DJOB_CORE) -o build/display-job-direct-finish-native
	build/display-job-direct-finish-native > build/display-job-direct-finish.snapshots
	cmp build/display-job-direct-finish.snapshots build/display-job-final-reference.snapshots
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -DDISPLAY_JOB_DIRECT_FINISH $(DJOB_FLAGS) tools/display_job_final_snapshot.c tests/display_job_final_cases.c $(DJOB_CORE) -o build/display-job-direct-finish-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/display-job-direct-finish-sanitize > build/display-job-direct-finish-sanitize.snapshots
	cmp build/display-job-direct-finish-sanitize.snapshots build/display-job-final-reference.snapshots
