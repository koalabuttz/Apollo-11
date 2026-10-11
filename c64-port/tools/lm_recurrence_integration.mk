# SPDX-License-Identifier: MIT
include tools/lm_recurrence_bridges.mk
LM_RECURRENCE_GUARD_CORE = src/agc_word.c src/waitlist.c src/lm_idle_start.c src/lm_taskover_resume.c src/lm_waitlist_dispatch.c tests/lm_idle_start_cases.c tests/lm_taskover_resume_cases.c tests/lm_waitlist_dispatch_cases.c
all: build/test-lm-recurrence-port
build/test-lm-recurrence-port: tests/test_lm_recurrence_bridge.c $(LM_RECURRENCE_GUARD_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
test: test-lm-recurrence-port
test-lm-recurrence-port: build/test-lm-recurrence-port
	./build/test-lm-recurrence-port
	python3 tools/lm_recurrence_port_host.py --cc "$(CC)"
sanitize: sanitize-lm-recurrence-port
sanitize-lm-recurrence-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_lm_recurrence_bridge.c $(LM_RECURRENCE_GUARD_CORE) -o build/test-lm-recurrence-port-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-recurrence-port-san
	python3 tools/lm_recurrence_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-lm-recurrence-port sanitize-lm-recurrence-port
