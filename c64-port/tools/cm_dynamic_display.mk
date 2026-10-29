# SPDX-License-Identifier: MIT
CM_DDISPLAY_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -ffunction-sections -Wl,--gc-sections
CM_DDISPLAY_CORE = src/agc_multiword.c src/agc_word.c src/cm_basic_finish.c src/cm_basic_priority.c src/cm_basic_wake.c src/cm_clockjob.c src/cm_dispatch.c src/cm_display_control.c src/cm_dynamic_display.c src/cm_dyndisp.c src/cm_dyndisp_route.c src/cm_executive.c src/cm_formatter.c src/cm_makeplay.c src/cm_native_image.c src/cm_nv85.c src/cm_nv85_route.c src/cm_pinball.c src/cm_powered_axis.c src/cm_powered_interface.c src/cm_powered_trig.c src/cm_rotation.c src/cm_scale.c src/dsky_decimal.c src/dsky_pinball.c src/dsky_scale.c src/dsky_two_digit.c src/gimbal_stage.c src/guidance_product.c src/guidance_shift.c src/guidance_trig.c src/matrix_native.c tests/cm_clockjob_cases.c tests/cm_dynamic_display_cases.c tests/cm_dyndisp_apply.c tests/cm_dyndisp_cases.c tests/cm_nv85_apply.c tests/cm_nv85_cases.c tests/cm_trig_cases.c tests/matrix_cases.c
cm_dynamic_display-host:
	$(CC) -O2 $(CM_DDISPLAY_FLAGS) $(CM_DDISPLAY_CORE) tests/test_cm_dynamic_display.c -o build/test-cm-dynamic-display
	build/test-cm-dynamic-display
cm_dynamic_display-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(CM_DDISPLAY_FLAGS) $(CM_DDISPLAY_CORE) tests/test_cm_dynamic_display.c -o build/test-cm-dynamic-display-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-dynamic-display-sanitize
	python3 tools/cm_dynamic_display_sanitize_stream.py
cm_dynamic_display-differential:
	python3 tools/cm_dynamic_display_differential.py
.PHONY: cm_dynamic_display-host cm_dynamic_display-sanitize cm_dynamic_display-differential
