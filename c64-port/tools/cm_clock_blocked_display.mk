# SPDX-License-Identifier: MIT
CM_CLOCK_BLOCKED_FLAGS = -DAPOLLO_FIXED_CM_ONLY -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_CLOCK_BLOCKED_CORE = src/agc_multiword.c src/agc_word.c src/cm_admission_image.c src/cm_basic_finish.c src/cm_basic_priority.c src/cm_basic_sleep.c src/cm_basic_wake.c src/cm_clock_display_builtin.c src/cm_clock_display_mission.c src/cm_clock_mission.c src/cm_clock_mission_builtin.c src/cm_clock_blocked_display.c src/cm_display_rest.c src/dsky_nv_admission.c src/dsky_nv_busy.c src/cm_clockjob.c src/cm_clocktask.c src/cm_danzig.c src/cm_decimal_pair.c src/cm_delay_image.c src/cm_dispatch.c src/cm_display_control.c src/cm_entry_tasks.c src/cm_executive.c src/cm_formatter.c src/cm_idle_dispatch.c src/cm_init_display.c src/cm_interbank.c src/cm_longcall.c src/cm_makeplay.c src/cm_makeplay_sleep.c src/cm_minutes_seconds.c src/cm_mission_builtin.c src/cm_mission_loop.c src/cm_native_image.c src/cm_normal_display.c src/cm_normal_display_route.c src/cm_nv85.c src/cm_nv85_route.c src/cm_phase.c src/cm_pinball.c src/cm_priority_complete.c src/cm_quictrig.c src/cm_restart_select.c src/cm_restart_timing.c src/cm_scale.c src/cm_service_builtin.c src/cm_service_loop.c src/cm_svct3.c src/cm_task_interrupt.c src/cm_task_transfer.c src/cm_tig5.c src/cm_waitlist.c src/cm_waitlist_image.c src/dsky_decimal.c src/dsky_pinball.c src/dsky_scale.c src/dsky_two_digit.c src/fixed_data.c src/guidance_product.c tests/cm_clock_display_mission_cases.c tests/cm_clock_mission_cases.c tests/cm_clockjob_cases.c tests/cm_mission_loop_cases.c tests/cm_normal_display_cases.c tests/cm_nv85_apply.c tests/cm_nv85_cases.c tests/cm_service_loop_cases.c tests/cm_trig_cases.c tests/cm_clock_blocked_display_check.c
cm-clock-blocked-host:
	$(CC) -O2 $(CM_CLOCK_BLOCKED_FLAGS) tests/test_cm_clock_blocked_display.c $(CM_CLOCK_BLOCKED_CORE) -o build/test-cm-clock-blocked-display
	build/test-cm-clock-blocked-display
cm-clock-blocked-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(filter-out -Wconversion,$(CM_CLOCK_BLOCKED_FLAGS)) tools/cm_clock_blocked_display_diagnostic.c $(CM_CLOCK_BLOCKED_CORE) -Wl,-Map,build/cm_clock_blocked_display.map -o build/cm_clock_blocked_display.prg
cm-clock-blocked-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(CM_CLOCK_BLOCKED_FLAGS) tests/test_cm_clock_blocked_display.c $(CM_CLOCK_BLOCKED_CORE) -o build/test-cm-clock-blocked-display-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-clock-blocked-display-sanitize
	python3 tools/cm_clock_blocked_display_sanitize.py
all: build/test-cm-clock-blocked-display build/cm-clock-blocked-display-route-port
build/test-cm-clock-blocked-display: tests/test_cm_clock_blocked_display.c $(CM_CLOCK_BLOCKED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_CLOCK_BLOCKED_FLAGS) $^ -o $@
build/cm-clock-blocked-display-route-port: tools/cm_clock_blocked_display_snapshot.c $(CM_CLOCK_BLOCKED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_CLOCK_BLOCKED_FLAGS) $^ -o $@
test: cm-clock-blocked-host
sanitize: cm-clock-blocked-sanitize
.PHONY: cm-clock-blocked-host cm-clock-blocked-c64 cm-clock-blocked-sanitize
