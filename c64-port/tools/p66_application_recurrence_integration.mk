# SPDX-License-Identifier: MIT
P66_APPLICATION_RECURRENCE_INTEGRATION_CORE = src/agc_word.c src/executive.c src/waitlist.c src/p66_native.c src/p66_startup.c src/p66_rodtask_image.c src/lm_waitlist_dispatch.c src/lm_waitlist_irq_dispatch.c src/lm_taskover_resume.c src/lm_idle_start.c src/lm_clock.c src/p66_application_recurrence.c src/dsky_display.c tests/test_p66_recurrence.c
P66_APPLICATION_RECURRENCE_INTEGRATION_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
all: build/test-p66-recurrence-integrated
build/test-p66-recurrence-integrated: $(P66_APPLICATION_RECURRENCE_INTEGRATION_CORE) | build
	$(CC) $(P66_APPLICATION_RECURRENCE_INTEGRATION_FLAGS) -O2 $^ -o $@
test: test-p66-recurrence-integrated
test-p66-recurrence-integrated: build/test-p66-recurrence-integrated
	./build/test-p66-recurrence-integrated
	python3 tools/p66_recurrence_port_host.py
sanitize: sanitize-p66-recurrence-integrated
sanitize-p66-recurrence-integrated: | build
	$(CC) $(P66_APPLICATION_RECURRENCE_INTEGRATION_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(P66_APPLICATION_RECURRENCE_INTEGRATION_CORE) -o build/test-p66-recurrence-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-recurrence-integrated-san
	python3 tools/p66_recurrence_port_host.py --sanitize
.PHONY: test-p66-recurrence-integrated sanitize-p66-recurrence-integrated
