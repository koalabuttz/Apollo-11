# SPDX-License-Identifier: MIT
CM_CLOCK_MISSION_CORE = src/cm_service_loop.c src/cm_service_builtin.c src/cm_task_transfer.c src/cm_entry_tasks.c src/cm_clocktask.c src/cm_tig5.c src/cm_svct3.c src/cm_waitlist_image.c src/cm_admission_image.c src/cm_delay_image.c src/cm_longcall.c src/cm_restart_select.c src/cm_restart_timing.c src/cm_init_display.c src/cm_basic_priority.c src/cm_dispatch.c src/cm_executive.c src/cm_waitlist.c src/cm_phase.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c tests/cm_service_loop_cases.c src/cm_mission_loop.c src/cm_mission_builtin.c src/cm_task_interrupt.c src/cm_danzig.c src/cm_basic_finish.c tests/cm_mission_loop_cases.c tests/cm_clock_mission_cases.c src/cm_clock_mission.c src/cm_clock_mission_builtin.c src/cm_clockjob.c src/cm_quictrig.c src/cm_display_control.c src/cm_idle_dispatch.c
build/cm-clock-mission-native: $(CM_CLOCK_MISSION_CORE) platform/host/cm_clock_mission_snapshot.c include/cm_clock_mission.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_CLOCK_MISSION_CORE) platform/host/cm_clock_mission_snapshot.c -o $@
cm-clock-mission-differential: build/cm-clock-mission-native
	python3 tools/cm_clock_mission_differential.py

.PHONY: test-cm-clock-mission sanitize-cm-clock-mission
test: test-cm-clock-mission
test-cm-clock-mission: build/test-cm-clock-mission
	./build/test-cm-clock-mission
build/test-cm-clock-mission: $(CM_CLOCK_MISSION_CORE) tests/cm_clock_mission_check.c tests/test_cm_clock_mission.c tests/cm_clock_mission_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_CLOCK_MISSION_CORE) tests/cm_clock_mission_check.c tests/test_cm_clock_mission.c -o $@
sanitize: sanitize-cm-clock-mission
sanitize-cm-clock-mission: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_CLOCK_MISSION_CORE) tests/cm_clock_mission_check.c tests/test_cm_clock_mission.c -o build/test-cm-clock-mission-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-clock-mission-sanitize
