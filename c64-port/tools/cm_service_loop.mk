# SPDX-License-Identifier: MIT
CM_SERVICE_CORE = src/cm_service_loop.c src/cm_service_builtin.c src/cm_task_transfer.c src/cm_entry_tasks.c src/cm_clocktask.c src/cm_tig5.c src/cm_svct3.c src/cm_waitlist_image.c src/cm_admission_image.c src/cm_delay_image.c src/cm_longcall.c src/cm_restart_select.c src/cm_restart_timing.c src/cm_init_display.c src/cm_basic_priority.c src/cm_dispatch.c src/cm_executive.c src/cm_waitlist.c src/cm_phase.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c tests/cm_service_loop_cases.c
CM_SERVICE_HEADERS = include/cm_service_loop.h tests/cm_service_loop_cases.h tests/cm_service_loop_check.h tests/cm_service_loop_vectors.h
.PHONY: cm-service-loop-differential test-cm-service-loop sanitize-cm-service-loop c64-cm-service-loop
all: build/cm-service-loop-native build/test-cm-service-loop
build/cm-service-loop-native: $(CM_SERVICE_CORE) platform/host/cm_service_loop_snapshot.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_SERVICE_CORE) platform/host/cm_service_loop_snapshot.c -o $@
cm-service-loop-differential: build/cm-service-loop-native
	python3 tools/cm_service_loop_differential.py
build/test-cm-service-loop: $(CM_SERVICE_CORE) tests/cm_service_loop_check.c tests/test_cm_service_loop.c $(CM_SERVICE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_SERVICE_CORE) tests/cm_service_loop_check.c tests/test_cm_service_loop.c -o $@
test: test-cm-service-loop
test-cm-service-loop: build/test-cm-service-loop
	./build/test-cm-service-loop
sanitize: sanitize-cm-service-loop
sanitize-cm-service-loop: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_SERVICE_CORE) tests/cm_service_loop_check.c tests/test_cm_service_loop.c -o build/test-cm-service-loop-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-service-loop-sanitize
c64-cm-service-loop: build/cm-service-loop.prg
build/cm-service-loop.prg: $(CM_SERVICE_CORE) tests/cm_service_loop_check.c platform/c64/cm_service_loop_diagnostic.c $(CM_SERVICE_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_SERVICE_TASK_PROOF_ONLY=1 -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-service-loop.map $(CM_SERVICE_CORE) tests/cm_service_loop_check.c platform/c64/cm_service_loop_diagnostic.c -o $@
