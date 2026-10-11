# SPDX-License-Identifier: MIT
CM_PHASE_CLEANUP_CORE = src/cm_phase_cleanup.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_phase_cleanup_cases.c tests/cm_vac5_store_cases.c tests/cm_admission_complete_cases.c
build/cm-phase-cleanup-native: $(CM_PHASE_CLEANUP_CORE) platform/host/cm_phase_cleanup_snapshot.c include/cm_phase_cleanup.h | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_PHASE_CLEANUP_CORE) platform/host/cm_phase_cleanup_snapshot.c -o $@
cm-phase-cleanup-differential: build/cm-phase-cleanup-native prepare-cm-original-rope
	python3 tools/cm_phase_cleanup_differential.py
.PHONY: test-cm-phase-cleanup sanitize-cm-phase-cleanup
test: test-cm-phase-cleanup
test-cm-phase-cleanup: cm-phase-cleanup-differential
sanitize: sanitize-cm-phase-cleanup
sanitize-cm-phase-cleanup: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_PHASE_CLEANUP_CORE) platform/host/cm_phase_cleanup_snapshot.c -o build/cm-phase-cleanup-sanitize
	python3 tools/cm_phase_cleanup_sanitize.py
