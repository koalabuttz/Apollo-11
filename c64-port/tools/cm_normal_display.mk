# SPDX-License-Identifier: MIT
CM_NORMAL_DISPLAY_FLAGS = -DAPOLLO_FIXED_CM_ONLY -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_NORMAL_DISPLAY_CORE = src/cm_normal_display.c src/fixed_data.c src/cm_formatter.c src/cm_pinball.c src/dsky_two_digit.c src/dsky_decimal.c src/dsky_pinball.c src/agc_word.c src/agc_multiword.c tests/cm_normal_display_cases.c tests/cm_nv85_cases.c tests/cm_clockjob_cases.c tests/cm_trig_cases.c tests/cm_normal_display_check.c
cm_normal_display-host:
	$(CC) -O2 $(CM_NORMAL_DISPLAY_FLAGS) tests/test_cm_normal_display.c $(CM_NORMAL_DISPLAY_CORE) -o build/test-cm-normal-display
	build/test-cm-normal-display
cm_normal_display-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(CM_NORMAL_DISPLAY_FLAGS) tests/test_cm_normal_display.c $(CM_NORMAL_DISPLAY_CORE) -o build/test-cm-normal-display-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-normal-display-sanitize
	python3 tools/cm_normal_display_sanitize_stream.py
cm_normal_display-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_NORMAL_DISPLAY_FLAGS) tools/cm_normal_display_diagnostic.c $(CM_NORMAL_DISPLAY_CORE) -Wl,-Map,build/cm_normal_display.map -o build/cm_normal_display.prg
cm_normal_display-differential:
	python3 tools/cm_normal_display_differential.py
.PHONY: cm_normal_display-host cm_normal_display-sanitize cm_normal_display-c64 cm_normal_display-differential

all: build/test-cm-normal-display
build/test-cm-normal-display: tests/test_cm_normal_display.c $(CM_NORMAL_DISPLAY_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_NORMAL_DISPLAY_FLAGS) $^ -o $@
test: cm_normal_display-host
sanitize: cm_normal_display-sanitize

CM_NORMAL_DISPLAY_ROUTE_CORE = src/agc_multiword.c src/agc_word.c src/cm_basic_finish.c src/cm_basic_priority.c src/cm_basic_wake.c src/cm_clockjob.c src/cm_decimal_pair.c src/cm_dispatch.c src/cm_display_control.c src/cm_executive.c src/cm_formatter.c src/cm_interbank.c src/cm_makeplay.c src/cm_minutes_seconds.c src/cm_native_image.c src/cm_normal_display.c src/cm_normal_display_route.c src/cm_nv85.c src/cm_nv85_route.c src/cm_pinball.c src/cm_scale.c src/dsky_decimal.c src/dsky_pinball.c src/dsky_scale.c src/dsky_two_digit.c src/fixed_data.c src/guidance_product.c tests/cm_clockjob_cases.c tests/cm_normal_display_cases.c tests/cm_nv85_apply.c tests/cm_nv85_cases.c tests/cm_trig_cases.c
all: build/cm-normal-display-route-port
build/cm-normal-display-route-port: tools/cm_normal_display_snapshot.c $(CM_NORMAL_DISPLAY_ROUTE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_NORMAL_DISPLAY_FLAGS) $^ -o $@
