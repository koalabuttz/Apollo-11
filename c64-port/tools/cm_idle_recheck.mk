# SPDX-License-Identifier: MIT
CM_IDLE_RECHECK_CORE = src/cm_idle_recheck.c src/cm_idle_dispatch.c src/cm_dispatch.c src/cm_native_image.c src/cm_executive.c src/agc_word.c tests/cm_idle_recheck_cases.c tests/cm_context_change_cases.c tests/cm_admission_complete_cases.c tests/cm_executive_cases.c
build/cm-idle-recheck-native: $(CM_IDLE_RECHECK_CORE) platform/host/cm_idle_recheck_snapshot.c include/cm_idle_recheck.h tests/cm_idle_recheck_cases.h tools/cm_priority_full_dump.h tools/cm_alarm2_image_dump.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IDLE_RECHECK_CORE) platform/host/cm_idle_recheck_snapshot.c -o $@
build/test-cm-idle-recheck: $(CM_IDLE_RECHECK_CORE) tests/test_cm_idle_recheck.c include/cm_idle_recheck.h tests/cm_idle_recheck_cases.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IDLE_RECHECK_CORE) tests/test_cm_idle_recheck.c -o $@
.PHONY: test-cm-idle-recheck sanitize-cm-idle-recheck cm-idle-recheck-differential
test: test-cm-idle-recheck
test-cm-idle-recheck: cm-idle-recheck-differential build/test-cm-idle-recheck
	./build/test-cm-idle-recheck
cm-idle-recheck-differential: build/cm-idle-recheck-native prepare-cm-original-rope
	python3 tools/cm_idle_recheck_differential.py
sanitize: sanitize-cm-idle-recheck
sanitize-cm-idle-recheck: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(CM_IDLE_RECHECK_CORE) platform/host/cm_idle_recheck_snapshot.c -o build/cm-idle-recheck-sanitize
	python3 tools/cm_idle_recheck_sanitize.py
