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
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RK_LM_FLAGS) -DRK_SEAMS=1 $(RK_LM_SRC) tests/test_resident_kernel.c -o $@
build/test-resident-kernel-cm: $(RK_CM_SRC) tests/test_resident_kernel.c $(RK_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RK_CM_FLAGS) -DRK_SEAMS=1 $(RK_CM_SRC) tests/test_resident_kernel.c -o $@
test-resident-kernel: build/test-resident-kernel-lm build/test-resident-kernel-cm resident-kernel-labels
	./build/test-resident-kernel-lm
	./build/test-resident-kernel-cm
sanitize-resident-kernel: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(RK_LM_FLAGS) -DRK_SEAMS=1 -fsanitize=address,undefined -fno-sanitize-recover=all $(RK_LM_SRC) tests/test_resident_kernel.c -o build/test-resident-kernel-lm-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(RK_CM_FLAGS) -DRK_SEAMS=1 -fsanitize=address,undefined -fno-sanitize-recover=all $(RK_CM_SRC) tests/test_resident_kernel.c -o build/test-resident-kernel-cm-sanitize
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
# The thin kernel in the linked ABI 2 flight resident (docs/RESIDENT-FLIGHT.md):
# each mission's flight resident with its per-component sizes, and per part a
# check program (the same resident) that runs the C64 rows from case modules
# in an REU store.
RF_SRC = platform/c64/resident_boundary_module.c platform/c64/resident_boundary_words_lm.h platform/c64/resident_boundary_words_cm.h src/sp_routines.c include/sp_routines.h src/resident_rupt.c include/resident_rupt.h tests/resident_rupt_cases.c tests/resident_rupt_cases.h tests/resident_rupt_c64_rows.h platform/c64/resident_flight_rupt_cases.c platform/c64/resident_flight.c platform/c64/resident_flight.h platform/c64/resident_flight.ld platform/c64/resident_flight_main.c platform/c64/resident_flight_check.c platform/c64/resident_flight_check.h platform/c64/resident_flight_cases.c platform/c64/resident_flight_data.c platform/c64/overlay_v2_irq.S platform/c64/overlay_v2_entry.S platform/c64/overlay_reu_platform.c src/overlay_v2.c src/overlay_reu.c src/overlay_runtime.c src/lm_single_trig.c src/cm_quictrig.c src/cm_single_return.c src/lm_restart_routines.c src/lm_restart_select.c docs/overlay-v2-services.json tests/lm_kernel_c64_rows.h tests/cm_kernel_c64_rows.h tests/resident_kernel_cases.c
RF_CHECKS = $(foreach m,lm cm,$(foreach p,1 2 3 4,build/resident-flight/$(m)-check-$(p).prg))
# The boundary-word tables (platform/c64/resident_boundary_words_<m>.h) must
# match the routine tables, the rope and, when built, the listings.
.PHONY: resident-boundary-words
test: resident-boundary-words
resident-boundary-words: | build
	python3 tools/resident_boundary_words.py --cc $(CC) --check
.PHONY: c64-resident-flight vice-resident-flight resident-flight-size
c64-resident-flight: $(RK_LM_SRC) $(RK_CM_SRC) $(RK_HDR) $(RF_SRC) tools/resident_flight_build.py | build
	python3 tools/resident_flight_build.py --cc $(MOS_CC)
vice-resident-flight: c64-resident-flight
	for m in lm cm; do for p in 1 2 3 4; do python3 tools/vice_run.py build/resident-flight/$$m-check-$$p.prg --reu build/resident-flight/$$m.reu --cycles 3000000000 --timeout 1200 || exit 1; done; done
resident-flight-size: c64-resident-flight
	cat build/resident-flight/sizes.json
# SPSIN/SPCOS on the transfer ABI (src/sp_routines.c), against the frozen
# yaAGC full-return states of lm-single-trig and cm-single-return.
SP_LM_SRC = src/sp_routines.c src/lm_single_trig.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_single_trig_cases.c tests/frozen_words.c
SP_CM_SRC = src/sp_routines.c src/cm_single_return.c src/cm_quictrig.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_single_return_cases.c tests/cm_trig_cases.c tests/frozen_words.c
SP_HDR = include/sp_routines.h include/lm_single_trig.h include/cm_single_return.h tests/lm_single_trig_vectors.h tests/cm_single_return_vectors.h
.PHONY: test-sp-routines sanitize-sp-routines
all: build/test-sp-routines-lm build/test-sp-routines-cm
test: test-sp-routines
sanitize: sanitize-sp-routines
build/test-sp-routines-lm: $(SP_LM_SRC) $(SP_HDR) tests/test_sp_routines.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DRK_LM=1 $(SP_LM_SRC) tests/test_sp_routines.c -o $@
build/test-sp-routines-cm: $(SP_CM_SRC) $(SP_HDR) tests/test_sp_routines.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DRK_CM=1 $(SP_CM_SRC) tests/test_sp_routines.c -o $@
test-sp-routines: build/test-sp-routines-lm build/test-sp-routines-cm
	./build/test-sp-routines-lm
	./build/test-sp-routines-cm
sanitize-sp-routines: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -DRK_LM=1 -fsanitize=address,undefined -fno-sanitize-recover=all $(SP_LM_SRC) tests/test_sp_routines.c -o build/test-sp-routines-lm-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -DRK_CM=1 -fsanitize=address,undefined -fno-sanitize-recover=all $(SP_CM_SRC) tests/test_sp_routines.c -o build/test-sp-routines-cm-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sp-routines-lm-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sp-routines-cm-sanitize
# The flight resident's AGC timers and interrupt entry (src/resident_rupt.c)
# against yaAGC (tests/resident_rupt_vectors.h, frozen by
# tools/resident_rupt_vectors.py), with T3RUPT run through the kernel to
# RESUME. Reads the unmodified ropes in evidence/.
RR_SRC = src/resident_rupt.c src/agc_image_ops.c src/agc_multiword.c src/agc_hardware_divide.c src/agc_word.c src/agc_transfer.c tests/resident_rupt_cases.c tests/test_resident_rupt.c
RR_HDR = include/resident_rupt.h include/resident_kernel.h tests/resident_rupt_cases.h tests/resident_rupt_vectors.h src/resident_kernel_body.inc
.PHONY: test-resident-rupt sanitize-resident-rupt
all: build/test-resident-rupt-lm build/test-resident-rupt-cm
test: test-resident-rupt
sanitize: sanitize-resident-rupt
build/test-resident-rupt-lm: $(RR_SRC) $(RR_HDR) src/lm_kernel.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DRK_LM=1 $(RR_SRC) src/lm_kernel.c -o $@
build/test-resident-rupt-cm: $(RR_SRC) $(RR_HDR) src/cm_kernel.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DRK_CM=1 $(RR_SRC) src/cm_kernel.c -o $@
test-resident-rupt: build/test-resident-rupt-lm build/test-resident-rupt-cm
	./build/test-resident-rupt-lm evidence/fixed-data-lm-original.rope.bin
	./build/test-resident-rupt-cm evidence/fixed-data-cm-original.rope.bin
sanitize-resident-rupt: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -DRK_LM=1 -fsanitize=address,undefined -fno-sanitize-recover=all $(RR_SRC) src/lm_kernel.c -o build/test-resident-rupt-lm-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -DRK_CM=1 -fsanitize=address,undefined -fno-sanitize-recover=all $(RR_SRC) src/cm_kernel.c -o build/test-resident-rupt-cm-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-resident-rupt-lm-sanitize evidence/fixed-data-lm-original.rope.bin
	ASAN_OPTIONS=detect_leaks=0 ./build/test-resident-rupt-cm-sanitize evidence/fixed-data-cm-original.rope.bin
