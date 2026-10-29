# SPDX-License-Identifier: MIT
CC ?= cc
RECURRENCE_GUARD_SOURCES = src/agc_word.c src/executive.c src/waitlist.c src/p66_native.c src/p66_startup.c src/p66_rodtask_image.c src/lm_waitlist_dispatch.c src/lm_waitlist_irq_dispatch.c src/lm_taskover_resume.c src/lm_idle_start.c src/lm_clock.c src/p66_application_recurrence.c src/dsky_display.c tests/test_p66_recurrence.c
RECURRENCE_GUARD_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
all: build/test-p66-recurrence
build/test-p66-recurrence: $(RECURRENCE_GUARD_SOURCES)
	$(CC) $(RECURRENCE_GUARD_FLAGS) -O2 $^ -o $@
test: all
	./build/test-p66-recurrence
	python3 tools/p66_recurrence_port_host.py
sanitize:
	$(CC) $(RECURRENCE_GUARD_FLAGS) -O1 -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(RECURRENCE_GUARD_SOURCES) -o build/test-p66-recurrence-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-recurrence-sanitize
	python3 tools/p66_recurrence_port_host.py --sanitize
reference:
	python3 tools/p66_recurrence_live_differential.py
	python3 tools/p66_recurrence_live_differential.py --sanitize
clock-reference:
	$(CC) -std=c99 -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections -DP66_NATIVE_DISPATCH_ONLY -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/lm_clock_reference_test.c src/agc_word.c src/lm_clock.c tests/scenarios.c deps/virtualagc/yaAGC/agc_engine.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o build/lm-clock-reference-current
	./build/lm-clock-reference-current
c64:
	python3 tools/p66_recurrence_live_build.py --cc "$(MOS_CC)" --c1541 "$(C1541)"
vice-test: all
	./build/test-p66-recurrence
	python3 tools/p66_recurrence_live_vice_test.py --vice "$(VICE)" --rom-dir "$(C64_ROM_DIR)" --data-dir "$(VICE_DATA_DIR)"
.PHONY: all test sanitize reference clock-reference c64 vice-test
