# SPDX-License-Identifier: MIT
CM_ABORT_LIFECYCLE_CORE = src/cm_abort_lifecycle.c src/cm_abort_snapshot.c src/cm_vac5_store.c src/cm_alarm_record.c src/cm_abort_recovery.c src/cm_abort_whimper.c src/cm_enema_image.c src/cm_enema.c src/cm_restart.c src/service_routines.c src/cm_phase_cleanup.c src/cm_interbank_extended.c src/cm_fixed_compat.c src/cm_native_image.c src/agc_word.c src/cm_executive.c src/cm_waitlist.c tests/cm_abort_lifecycle_cases.c tests/cm_enema_image_cases.c tests/cm_enema_cases.c tests/cm_restart_cases.c
build/cm-abort-lifecycle-native: $(CM_ABORT_LIFECYCLE_CORE) platform/host/cm_abort_lifecycle_snapshot.c include/cm_abort_lifecycle.h | build
	$(CC) $(CPPFLAGS) -DCM_RESTART_INPUT_ONLY -Itools $(CFLAGS) $(CM_ABORT_LIFECYCLE_CORE) platform/host/cm_abort_lifecycle_snapshot.c -o $@
cm-abort-lifecycle-differential: build/cm-abort-lifecycle-native prepare-cm-original-rope
	python3 tools/cm_abort_lifecycle_differential.py
.PHONY: test-cm-abort-lifecycle sanitize-cm-abort-lifecycle
test: test-cm-abort-lifecycle
test-cm-abort-lifecycle: cm-abort-lifecycle-differential
sanitize: sanitize-cm-abort-lifecycle
sanitize-cm-abort-lifecycle: | build
	$(CC) $(CPPFLAGS) -DCM_RESTART_INPUT_ONLY -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ABORT_LIFECYCLE_CORE) platform/host/cm_abort_lifecycle_snapshot.c -o build/cm-abort-lifecycle-sanitize
	python3 tools/cm_abort_lifecycle_sanitize.py
