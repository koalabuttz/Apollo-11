# SPDX-License-Identifier: MIT
DSKY_MODE_JOB_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_MODE_JOB_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_two_digit.c src/cm_formatter.c src/dsky_mode_job.c tests/p66_bias_cases.c tests/dsky_mode_job_cases.c
DSKY_MODE_JOB_TEST = tests/dsky_mode_job_check.c tests/test_dsky_mode_job.c
all: build/test-dsky-mode-job
build/test-dsky-mode-job: $(DSKY_MODE_JOB_CORE) $(DSKY_MODE_JOB_TEST) tests/dsky_mode_job_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_MODE_JOB_FLAGS) $(DSKY_MODE_JOB_CORE) $(DSKY_MODE_JOB_TEST) -o $@
test-dsky-mode-job: build/test-dsky-mode-job
	build/test-dsky-mode-job
sanitize-dsky-mode-job:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_MODE_JOB_FLAGS) $(DSKY_MODE_JOB_CORE) $(DSKY_MODE_JOB_TEST) -o build/test-dsky-mode-job-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-mode-job-sanitize
c64-dsky-mode-job:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_MODE_JOB_TARGET_LM $(DSKY_MODE_JOB_FLAGS) $(DSKY_MODE_JOB_CORE) tests/dsky_mode_job_check.c tools/dsky_mode_job_diagnostic.c -Wl,-Map,build/dsky_mode_job-lm.map -o build/dsky_mode_job-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_MODE_JOB_TARGET_CM $(DSKY_MODE_JOB_FLAGS) $(DSKY_MODE_JOB_CORE) tests/dsky_mode_job_check.c tools/dsky_mode_job_diagnostic.c -Wl,-Map,build/dsky_mode_job-cm.map -o build/dsky_mode_job-cm.prg
differential-dsky-mode-job:
	python3 tools/dsky_mode_job_differential.py
	python3 tools/freeze_dsky_mode_job.py
test: test-dsky-mode-job
sanitize: sanitize-dsky-mode-job
.PHONY: all test sanitize test-dsky-mode-job sanitize-dsky-mode-job c64-dsky-mode-job differential-dsky-mode-job
