# SPDX-License-Identifier: MIT
# Milestone lm-exec-jobs (lane A): Luminary099 SPVAC, CHANG1/CHANG2, JOBSLEEP,
# JOBWAKE, PRIOCHNG, ENDOFJOB, FIXDELAY, VARDELAY, TASKOVER and the T3RUPT
# vector as native routines of the control-transfer ABI (lm_exec_job_routines,
# lm_exec_task_routines). The C64 diagnostic is three programs.
LM_EXJ_BODIES = src/lm_admission_image.c src/lm_basic_sleep.c src/lm_basic_wake.c src/lm_basic_priority.c src/lm_basic_finish.c src/lm_basic_change.c src/lm_interpreter_change.c src/lm_delay_image.c src/lm_taskover_resume.c src/lm_waitlist_dispatch.c src/lm_waitlist_irq_dispatch.c
LM_EXJ_SRC = src/lm_exec_jobs.c src/lm_exec_tasks.c src/lm_exec_routines.c src/lm_waitlist_image.c $(LM_EXJ_BODIES) src/waitlist.c src/executive.c src/p66_native.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_admission_image_cases.c tests/lm_basic_change_cases.c tests/lm_interpreter_change_cases.c tests/lm_sleep_cases.c tests/lm_wake_cases.c tests/lm_priority_cases.c tests/lm_finish_cases.c tests/lm_delay_image_cases.c tests/lm_waitlist_image_cases.c tests/lm_taskover_resume_cases.c tests/lm_waitlist_dispatch_cases.c tests/throttle_bridge_cases.c tests/lm_exec_jobs_cases.c
LM_EXJ_HDR = include/lm_exec_routines.h include/agc_transfer.h src/lm_exec_sites.inc tests/lm_exec_jobs_cases.h
LM_EXJ_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-exec-jobs sanitize-lm-exec-jobs c64-lm-exec-jobs vice-lm-exec-jobs lm-exec-jobs-differential
all: build/test-lm-exec-jobs
test: test-lm-exec-jobs
sanitize: sanitize-lm-exec-jobs
build/test-lm-exec-jobs: $(LM_EXJ_SRC) tests/test_lm_exec_jobs.c tests/lm_exec_jobs_vectors.h $(LM_EXJ_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_EXJ_FLAGS) $(LM_EXJ_SRC) tests/test_lm_exec_jobs.c -o $@
test-lm-exec-jobs: build/test-lm-exec-jobs
	python3 tools/agc_sites_generate.py lm-exec --check
	./build/test-lm-exec-jobs
sanitize-lm-exec-jobs: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_EXJ_FLAGS) -fsanitize=address,undefined $(LM_EXJ_SRC) tests/test_lm_exec_jobs.c -o build/test-lm-exec-jobs-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-exec-jobs-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-exec-jobs-native: $(LM_EXJ_SRC) platform/host/lm_exec_jobs_snapshot.c $(LM_EXJ_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_EXJ_FLAGS) $(LM_EXJ_SRC) platform/host/lm_exec_jobs_snapshot.c -o $@
lm-exec-jobs-differential: build/lm-exec-jobs-native
	python3 tools/lm_exec_jobs_freeze.py
	git diff --exit-code -- tests/lm_exec_jobs_vectors.h tests/lm_exec_jobs_c64_vectors.h
c64-lm-exec-jobs: build/lm-exec-jobs-1.prg build/lm-exec-jobs-2.prg build/lm-exec-jobs-3.prg
build/lm-exec-jobs-%.prg: $(LM_EXJ_SRC) platform/c64/lm_exec_jobs_diagnostic.c tests/lm_exec_jobs_c64_vectors.h tests/lm_exec_jobs_c64_rows.h $(LM_EXJ_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -DP66_NATIVE_DISPATCH_ONLY -DLM_EXJ_PART=$* -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-exec-jobs-$*.map $(LM_EXJ_SRC) platform/c64/lm_exec_jobs_diagnostic.c -o $@
vice-lm-exec-jobs: build/lm-exec-jobs-1.prg build/lm-exec-jobs-2.prg build/lm-exec-jobs-3.prg
	python3 tools/vice_run.py build/lm-exec-jobs-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-exec-jobs-2.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-exec-jobs-3.prg --cycles 3000000000 --timeout 1200
