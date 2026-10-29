# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
RCS_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
RCS_CORE = src/lm_rcs_monitor.c src/executive.c src/agc_word.c tests/lm_rcs_monitor_cases.c
all: build/lm-rcs-monitor-native build/lm-rcs-monitor-reference
build/lm-rcs-monitor-native: tools/lm_rcs_monitor_snapshot.c $(RCS_CORE)
	gcc -O2 $(RCS_FLAGS) tools/lm_rcs_monitor_snapshot.c $(RCS_CORE) -o $@
build/lm-rcs-monitor-reference: tools/lm_rcs_monitor_reference.c tests/lm_rcs_monitor_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/lm_rcs_monitor_reference.c tests/lm_rcs_monitor_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/lm-rcs-monitor-reference deps/lm-rcs-monitor-fixture/MAIN.agc.bin | gzip -n -9 > build/lm-rcs-monitor-reference.snapshots.gz
	build/lm-rcs-monitor-native | gzip -n -9 > build/lm-rcs-monitor-native.snapshots.gz
	cmp build/lm-rcs-monitor-reference.snapshots.gz build/lm-rcs-monitor-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(RCS_FLAGS) tools/lm_rcs_monitor_snapshot.c $(RCS_CORE) -o build/lm-rcs-monitor-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-rcs-monitor-sanitize | gzip -n -9 > build/lm-rcs-monitor-sanitize.snapshots.gz
	cmp build/lm-rcs-monitor-reference.snapshots.gz build/lm-rcs-monitor-sanitize.snapshots.gz
.PHONY: all test sanitize

check-host:
	gcc -O2 $(RCS_FLAGS) tests/test_lm_rcs_monitor.c tests/frozen_words.c $(RCS_CORE) -o build/test-lm-rcs-monitor
	build/test-lm-rcs-monitor
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DLM_RCS_MONITOR_CHECK_ONLY $(RCS_FLAGS) tests/test_lm_rcs_monitor.c tests/frozen_words.c tools/lm_rcs_monitor_diagnostic.c $(RCS_CORE) -Wl,-Map,build/lm-rcs-monitor0.map -o build/lm-rcs-monitor0.prg
vice-test: c64
	python tools/lm_rcs_monitor_vice_test.py --phase 0 --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: check-host c64 vice-test
