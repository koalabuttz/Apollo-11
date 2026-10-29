# SPDX-License-Identifier: MIT
# Milestone cm-exec-jobs (lane A): Comanche055 SPVACIN, SPVAC, CHANG1, CHANG2,
# JOBSLEEP, JOBWAKE, PRIOCHNG and ENDOFJOB as native routines of the
# control-transfer ABI (cm_exec_job_routines).
CM_EXJ_SRC = src/cm_exec_jobs.c src/cm_admission_zero.c src/cm_context_change.c src/cm_basic_sleep.c src/cm_wake_complete.c src/cm_priority_complete.c src/cm_basic_finish.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_exec_jobs_cases.c tests/cm_admission_zero_cases.c tests/cm_admission_complete_cases.c tests/cm_context_change_cases.c tests/cm_sleep_cases.c tests/cm_basic_finish_cases.c tests/cm_wake_complete_cases.c tests/cm_priority_complete_cases.c
CM_EXJ_HDR = include/cm_exec_routines.h include/agc_transfer.h tests/cm_exec_jobs_cases.h
CM_EXJ_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-cm-exec-jobs sanitize-cm-exec-jobs c64-cm-exec-jobs vice-cm-exec-jobs cm-exec-jobs-differential
all: build/test-cm-exec-jobs
test: test-cm-exec-jobs
sanitize: sanitize-cm-exec-jobs
build/test-cm-exec-jobs: $(CM_EXJ_SRC) tests/test_cm_exec_jobs.c tests/cm_exec_jobs_vectors.h $(CM_EXJ_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_EXJ_FLAGS) $(CM_EXJ_SRC) tests/test_cm_exec_jobs.c -o $@
test-cm-exec-jobs: build/test-cm-exec-jobs
	./build/test-cm-exec-jobs
sanitize-cm-exec-jobs: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(CM_EXJ_FLAGS) -fsanitize=address,undefined $(CM_EXJ_SRC) tests/test_cm_exec_jobs.c -o build/test-cm-exec-jobs-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-exec-jobs-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-exec-jobs-native: $(CM_EXJ_SRC) platform/host/cm_exec_jobs_snapshot.c $(CM_EXJ_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_EXJ_FLAGS) $(CM_EXJ_SRC) platform/host/cm_exec_jobs_snapshot.c -o $@
cm-exec-jobs-differential: build/cm-exec-jobs-native
	python3 tools/cm_exec_jobs_freeze.py
	git diff --exit-code -- tests/cm_exec_jobs_vectors.h tests/cm_exec_jobs_c64_vectors.h
c64-cm-exec-jobs: build/cm-exec-jobs.prg
build/cm-exec-jobs.prg: $(CM_EXJ_SRC) platform/c64/cm_exec_jobs_diagnostic.c tests/cm_exec_jobs_c64_vectors.h tests/cm_exec_jobs_c64_rows.h $(CM_EXJ_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(CM_EXJ_FLAGS) -Wl,-Map,build/cm-exec-jobs.map $(CM_EXJ_SRC) platform/c64/cm_exec_jobs_diagnostic.c -o $@
vice-cm-exec-jobs: build/cm-exec-jobs.prg
	python3 tools/vice_run.py build/cm-exec-jobs.prg --cycles 3000000000 --timeout 1200
