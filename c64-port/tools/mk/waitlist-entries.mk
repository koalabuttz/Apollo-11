# SPDX-License-Identifier: MIT
# Milestone waitlist-entries (lane A): WAITLIST entries reached from other
# files (T3RUPT, DLY2 -1, LM NOQBRSM +1) as transfer routines, against yaAGC.
WLE_SRC = src/lm_exec_jobs.c \
	src/lm_exec_tasks.c \
	src/lm_exec_routines.c \
	src/lm_waitlist_image.c \
	src/lm_admission_image.c \
	src/lm_basic_sleep.c \
	src/lm_basic_wake.c \
	src/lm_basic_priority.c \
	src/lm_basic_finish.c \
	src/lm_basic_change.c \
	src/lm_interpreter_change.c \
	src/lm_delay_image.c \
	src/lm_taskover_resume.c \
	src/lm_waitlist_dispatch.c \
	src/lm_waitlist_irq_dispatch.c \
	src/waitlist.c \
	src/executive.c \
	src/p66_native.c \
	src/agc_transfer.c \
	src/agc_word.c \
	src/agc_multiword.c \
	tests/lm_admission_image_cases.c \
	tests/lm_basic_change_cases.c \
	tests/lm_interpreter_change_cases.c \
	tests/lm_sleep_cases.c \
	tests/lm_wake_cases.c \
	tests/lm_priority_cases.c \
	tests/lm_finish_cases.c \
	tests/lm_delay_image_cases.c \
	tests/lm_waitlist_image_cases.c \
	tests/lm_taskover_resume_cases.c \
	tests/lm_waitlist_dispatch_cases.c \
	tests/throttle_bridge_cases.c \
	tests/lm_exec_jobs_cases.c \
	src/cm_exec_routines.c \
	src/cm_admission_zero.c \
	src/cm_waitlist_extended.c \
	src/cm_waitlist_internal.c \
	src/cm_task_transfer.c \
	src/cm_native_image.c \
	src/cm_executive.c \
	src/cm_waitlist.c \
	tests/cm_exec_services_cases.c \
	src/lm_t6rupt.c \
	src/agc_image_ops.c \
	src/agc_hardware_divide.c \
	tests/lm_t6rupt_cases.c \
	tests/service_transfer_cases.c \
	src/service_transfer.c \
	src/service_routines.c \
	tests/waitlist_entries_cases.c \
	tests/waitlist_entries_check.c
WLE_HDR = include/agc_transfer.h include/lm_exec_routines.h include/cm_exec_routines.h include/lm_t6rupt_routines.h \
	include/service_transfer.h tests/waitlist_entries_cases.h tests/waitlist_entries_check.h tests/waitlist_entries_vectors.h
WLE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-waitlist-entries sanitize-waitlist-entries c64-waitlist-entries vice-waitlist-entries waitlist-entries-differential
all: build/test-waitlist-entries
test: test-waitlist-entries
sanitize: sanitize-waitlist-entries
build/test-waitlist-entries: $(WLE_SRC) tests/test_waitlist_entries.c $(WLE_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WLE_FLAGS) $(WLE_SRC) tests/test_waitlist_entries.c -o $@
test-waitlist-entries: build/test-waitlist-entries
	./build/test-waitlist-entries
sanitize-waitlist-entries: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(WLE_FLAGS) -fsanitize=address,undefined $(WLE_SRC) tests/test_waitlist_entries.c -o build/test-waitlist-entries-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-waitlist-entries-sanitize
# Host-only oracle run (needs deps/virtualagc and both rope builds).
waitlist-entries-differential:
	python3 tools/waitlist_entries_freeze.py
	git diff --exit-code -- tests/waitlist_entries_vectors.h
WLE_PRGS = build/waitlist-entries-0.prg build/waitlist-entries-1.prg build/waitlist-entries-2.prg build/waitlist-entries-3.prg build/waitlist-entries-4.prg
c64-waitlist-entries: $(WLE_PRGS)
build/waitlist-entries-%.prg: $(WLE_SRC) platform/c64/waitlist_entries_diagnostic.c $(WLE_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -DWLE_ONLY=$* $(if $(filter 3,$*),-DCM_EXEC_DLY2_ONLY -DCM_SERVICE_DELAYJOB_ONLY) $(WLE_FLAGS) -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/waitlist-entries-$*.map $(WLE_SRC) platform/c64/waitlist_entries_diagnostic.c -o $@
vice-waitlist-entries: $(WLE_PRGS)
	for p in $(WLE_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1800 || exit 1; done
