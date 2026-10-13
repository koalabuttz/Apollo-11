# SPDX-License-Identifier: MIT
# Milestone resident-kernel (Resident kernel lane): the thin resident
# EXECUTIVE and WAITLIST of both missions (src/lm_kernel.c, src/cm_kernel.c,
# one body in src/resident_kernel_body.inc) on every case of the six
# certified service milestones, against their yaAGC vectors and the
# certified routines' final images.
RK_LM_CASES = src/lm_exec_routines.c src/lm_admission_image.c src/lm_waitlist_image.c src/waitlist.c src/executive.c src/p66_native.c src/lm_restart_select.c src/lm_restart_routines.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_restart_select_cases.c tests/lm_restart_routines_cases.c tests/lm_exec_services_cases.c src/lm_exec_jobs.c src/lm_exec_tasks.c src/lm_basic_sleep.c src/lm_basic_wake.c src/lm_basic_priority.c src/lm_basic_finish.c src/lm_basic_change.c src/lm_interpreter_change.c src/lm_delay_image.c src/lm_taskover_resume.c src/lm_waitlist_dispatch.c src/lm_waitlist_irq_dispatch.c tests/lm_admission_image_cases.c tests/lm_basic_change_cases.c tests/lm_interpreter_change_cases.c tests/lm_sleep_cases.c tests/lm_wake_cases.c tests/lm_priority_cases.c tests/lm_finish_cases.c tests/lm_delay_image_cases.c tests/lm_waitlist_image_cases.c tests/lm_taskover_resume_cases.c tests/lm_waitlist_dispatch_cases.c tests/throttle_bridge_cases.c tests/lm_exec_jobs_cases.c src/lm_exec_idle.c src/lm_idle_dispatch.c src/lm_super_dispatch.c src/lm_svct3.c tests/lm_exec_idle_cases.c tests/lm_idle_dispatch_cases.c tests/lm_super_dispatch_cases.c tests/lm_svct3_cases.c tests/lm_context_lifecycle_cases.c
RK_CM_CASES = src/cm_exec_routines.c src/cm_admission_zero.c src/cm_waitlist_extended.c src/cm_waitlist_internal.c src/cm_task_transfer.c src/cm_native_image.c src/cm_executive.c src/cm_waitlist.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_exec_services_cases.c src/cm_exec_jobs.c src/cm_context_change.c src/cm_basic_sleep.c src/cm_wake_complete.c src/cm_priority_complete.c src/cm_basic_finish.c src/cm_dispatch.c tests/cm_exec_jobs_cases.c tests/cm_admission_zero_cases.c tests/cm_admission_complete_cases.c tests/cm_context_change_cases.c tests/cm_sleep_cases.c tests/cm_basic_finish_cases.c tests/cm_wake_complete_cases.c tests/cm_priority_complete_cases.c src/cm_exec_idle.c src/cm_idle_dispatch.c src/cm_idle_recheck.c src/cm_super_dispatch.c src/cm_svct3.c tests/cm_exec_idle_cases.c tests/cm_idle_dispatch_cases.c tests/cm_idle_recheck_cases.c tests/cm_super_dispatch_cases.c tests/cm_svct3_cases.c
RK_LM_SRC = $(RK_LM_CASES) src/lm_kernel.c src/agc_image_ops.c tests/resident_kernel_cases.c
RK_CM_SRC = $(RK_CM_CASES) src/cm_kernel.c src/agc_image_ops.c tests/resident_kernel_cases.c
RK_HDR = include/resident_kernel.h include/agc_image_ops.h include/agc_transfer.h src/resident_kernel_body.inc src/lm_kernel_labels.inc src/cm_kernel_labels.inc tests/resident_kernel_cases.h
RK_LM_FLAGS = -DRK_LM=1 -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
RK_CM_FLAGS = -DRK_CM=1 -ffunction-sections -fdata-sections -Wl,--gc-sections
# C64: the kernel on the canonical image at $7000, everything else below it.
RK_MOS_FLAGS = -std=c99 -Oz -DRK_FIXED_IMAGE=1 -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -T platform/c64/resident_kernel.ld
.PHONY: test-resident-kernel sanitize-resident-kernel c64-resident-kernel vice-resident-kernel resident-kernel-labels resident-kernel-size resident-kernel-rows
all: build/test-resident-kernel-lm build/test-resident-kernel-cm
test: test-resident-kernel
sanitize: sanitize-resident-kernel
build/test-resident-kernel-lm: $(RK_LM_SRC) tests/test_resident_kernel.c $(RK_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RK_LM_FLAGS) $(RK_LM_SRC) tests/test_resident_kernel.c -o $@
build/test-resident-kernel-cm: $(RK_CM_SRC) tests/test_resident_kernel.c $(RK_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RK_CM_FLAGS) $(RK_CM_SRC) tests/test_resident_kernel.c -o $@
test-resident-kernel: build/test-resident-kernel-lm build/test-resident-kernel-cm resident-kernel-labels
	./build/test-resident-kernel-lm
	./build/test-resident-kernel-cm
sanitize-resident-kernel: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(RK_LM_FLAGS) -fsanitize=address,undefined -fno-sanitize-recover=all $(RK_LM_SRC) tests/test_resident_kernel.c -o build/test-resident-kernel-lm-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(RK_CM_FLAGS) -fsanitize=address,undefined -fno-sanitize-recover=all $(RK_CM_SRC) tests/test_resident_kernel.c -o build/test-resident-kernel-cm-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-resident-kernel-lm-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-resident-kernel-cm-sanitize
# The label macros are generated from the listings (needs the listing build).
resident-kernel-labels:
	@if [ -f build/fixed-data/Luminary099/assembly.lst ] && [ -f build/fixed-data/Comanche055/assembly.lst ]; then python3 tools/resident_kernel_labels.py --check; else echo "SKIP: resident-kernel labels (no listings)"; fi
# C64 rows: the fewest cases taking every kernel branch arc the host corpus
# takes (gcov), frozen with their yaAGC digests (host only; slow).
resident-kernel-rows: | build
	python3 tools/resident_kernel_rows.py lm $(RK_LM_FLAGS) -- $(RK_LM_SRC)
	python3 tools/resident_kernel_rows.py cm $(RK_CM_FLAGS) -- $(RK_CM_SRC)
# One program per module (LM jobs in lm-exec-jobs' three cohort parts), so
# each part's case generators fit below the $7000 image.
RK_LM_PARTS = 1 2-1 2-2 2-3 3
RK_CM_PARTS = 1 2 3
RK_PRGS = $(foreach p,$(RK_LM_PARTS),build/resident-kernel-lm-$(p).prg) $(foreach p,$(RK_CM_PARTS),build/resident-kernel-cm-$(p).prg)
rk_part = -DRK_PART=$(firstword $(subst -, ,$(1))) $(if $(word 2,$(subst -, ,$(1))),-DLM_EXJ_PART=$(word 2,$(subst -, ,$(1))))
c64-resident-kernel: $(RK_PRGS)
build/resident-kernel-lm-%.prg: $(RK_LM_SRC) platform/c64/resident_kernel_diagnostic.c platform/c64/resident_kernel.ld tests/lm_kernel_c64_rows.h $(RK_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(RK_MOS_FLAGS) $(RK_LM_FLAGS) $(call rk_part,$*) -Wl,-Map,build/resident-kernel-lm-$*.map $(RK_LM_SRC) platform/c64/resident_kernel_diagnostic.c -o $@
build/resident-kernel-cm-%.prg: $(RK_CM_SRC) platform/c64/resident_kernel_diagnostic.c platform/c64/resident_kernel.ld tests/cm_kernel_c64_rows.h $(RK_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(RK_MOS_FLAGS) $(RK_CM_FLAGS) $(call rk_part,$*) -Wl,-Map,build/resident-kernel-cm-$*.map $(RK_CM_SRC) platform/c64/resident_kernel_diagnostic.c -o $@
vice-resident-kernel: $(RK_PRGS)
	for p in $(RK_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
# The kernel alone, LTO + gc-sections, on the $7000 image (docs/RESIDENT-KERNEL.md).
resident-kernel-size: | build
	python3 tools/resident_kernel_size.py
