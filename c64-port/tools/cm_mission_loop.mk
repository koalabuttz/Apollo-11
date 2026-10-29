# SPDX-License-Identifier: MIT
CM_MISSION_CORE = src/cm_service_loop.c src/cm_service_builtin.c src/cm_task_transfer.c src/cm_entry_tasks.c src/cm_clocktask.c src/cm_tig5.c src/cm_svct3.c src/cm_waitlist_image.c src/cm_admission_image.c src/cm_delay_image.c src/cm_longcall.c src/cm_restart_select.c src/cm_restart_timing.c src/cm_init_display.c src/cm_basic_priority.c src/cm_dispatch.c src/cm_executive.c src/cm_waitlist.c src/cm_phase.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c tests/cm_service_loop_cases.c src/cm_mission_loop.c src/cm_mission_builtin.c src/cm_task_interrupt.c src/cm_danzig.c src/cm_basic_finish.c tests/cm_mission_loop_cases.c
build/cm-mission-loop-native: $(CM_MISSION_CORE) platform/host/cm_mission_loop_snapshot.c include/cm_mission_loop.h tests/cm_mission_loop_cases.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_MISSION_CORE) platform/host/cm_mission_loop_snapshot.c -o $@
cm-mission-loop-differential: build/cm-mission-loop-native
	python3 tools/cm_mission_loop_differential.py

test: test-cm-mission-loop
test-cm-mission-loop: build/test-cm-mission-loop
	./build/test-cm-mission-loop
build/test-cm-mission-loop: $(CM_MISSION_CORE) tests/cm_mission_loop_check.c tests/test_cm_mission_loop.c tests/cm_mission_loop_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_MISSION_CORE) tests/cm_mission_loop_check.c tests/test_cm_mission_loop.c -o $@
sanitize: sanitize-cm-mission-loop
sanitize-cm-mission-loop: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_MISSION_CORE) tests/cm_mission_loop_check.c tests/test_cm_mission_loop.c -o build/test-cm-mission-loop-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-mission-loop-sanitize
