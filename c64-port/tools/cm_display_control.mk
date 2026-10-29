# SPDX-License-Identifier: MIT
CMDCTRL_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CMDCTRL_CORE = src/agc_word.c src/cm_display_control.c src/cm_clockjob.c tests/cm_trig_cases.c tests/cm_clockjob_cases.c tests/cm_display_control_cases.c
cm_display_control-host:
	$(CC) -O2 $(CMDCTRL_FLAGS) tests/test_cm_display_control.c tests/cm_display_control_check.c $(CMDCTRL_CORE) -o build/test-cm-display-control
	build/test-cm-display-control
cm_display_control-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMDCTRL_FLAGS) tests/test_cm_display_control.c tests/cm_display_control_check.c $(CMDCTRL_CORE) -o build/test-cm-display-control-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-display-control-sanitize
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMDCTRL_FLAGS) tools/cm_display_control_snapshot.c $(CMDCTRL_CORE) -o build/cm-display-control-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-display-control-sanitize > build/cm-display-control-sanitize.snapshots
	cmp build/cm-display-control-sanitize.snapshots build/cm-display-control-reference.snapshots
cm_display_control-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CMDCTRL_FLAGS) tools/cm_display_control_diagnostic.c tests/cm_display_control_check.c $(CMDCTRL_CORE) -Wl,-Map,build/cm_display_control.map -o build/cm_display_control.prg
cm_display_control-differential:
	python3 tools/cm_display_control_differential.py
all: build/cm-display-control-native
build/cm-display-control-native: tools/cm_display_control_snapshot.c $(CMDCTRL_CORE) | build
	$(CC) -O2 $(CMDCTRL_FLAGS) $^ -o $@
.PHONY: cm_display_control-host cm_display_control-sanitize cm_display_control-c64 cm_display_control-differential
cm_display_control-full:
	python3 tools/cm_display_control_full_differential.py
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMDCTRL_FLAGS) tools/cm_display_control_full_snapshot.c $(CMDCTRL_CORE) src/cm_quictrig.c src/agc_multiword.c -o build/cm-display-control-full-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-display-control-full-sanitize > build/cm-display-control-full-sanitize.snapshots
	cmp build/cm-display-control-full-sanitize.snapshots build/cm-display-control-full-reference.snapshots
