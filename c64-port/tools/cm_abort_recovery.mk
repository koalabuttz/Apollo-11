# SPDX-License-Identifier: MIT
CM_ABORT_RECOVERY_CORE = src/cm_abort_recovery.c src/service_routines.c src/cm_phase_cleanup.c src/cm_interbank_extended.c src/cm_fixed_compat.c src/cm_native_image.c src/agc_word.c src/cm_executive.c src/cm_waitlist.c tests/cm_abort_recovery_cases.c tests/cm_enema_image_cases.c tests/cm_enema_cases.c tests/cm_restart_cases.c
build/cm-abort-recovery-native: $(CM_ABORT_RECOVERY_CORE) platform/host/cm_abort_recovery_snapshot.c include/cm_abort_recovery.h | build
	$(CC) $(CPPFLAGS) -DCM_RESTART_INPUT_ONLY -Itools $(CFLAGS) $(CM_ABORT_RECOVERY_CORE) platform/host/cm_abort_recovery_snapshot.c -o $@
cm-abort-recovery-differential: build/cm-abort-recovery-native prepare-cm-original-rope
	python3 tools/cm_abort_recovery_differential.py
.PHONY: test-cm-abort-recovery sanitize-cm-abort-recovery
test: test-cm-abort-recovery
test-cm-abort-recovery: cm-abort-recovery-differential
sanitize: sanitize-cm-abort-recovery
sanitize-cm-abort-recovery: | build
	$(CC) $(CPPFLAGS) -DCM_RESTART_INPUT_ONLY -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ABORT_RECOVERY_CORE) platform/host/cm_abort_recovery_snapshot.c -o build/cm-abort-recovery-sanitize
	python3 tools/cm_abort_recovery_sanitize.py
