# SPDX-License-Identifier: MIT
P66_DIRECT_PENDING_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests -ffunction-sections -DP66_NATIVE_DISPATCH_ONLY -DP66_DIRECT_LAUNCH_DISPATCH_ONLY
P66_DIRECT_PENDING_CORE = src/agc_word.c src/agc_multiword.c src/executive.c src/waitlist.c src/phase.c src/p66_startup.c src/p66_initialization.c src/p66_vertical_start.c src/p66_direct_launch.c src/p66_native.c src/native_dispatch.c src/irq_delivery.c tests/p66_bias_cases.c tests/p66_vertical_start_cases.c tests/p66_launch_cases.c tests/p66_direct_launch_cases.c
P66_DIRECT_PENDING_TEST = tests/p66_direct_pending_check.c tests/test_p66_direct_pending.c
all: build/test-p66-direct-pending
build/test-p66-direct-pending: $(P66_DIRECT_PENDING_CORE) $(P66_DIRECT_PENDING_TEST) tests/p66_direct_pending_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(P66_DIRECT_PENDING_FLAGS) $(P66_DIRECT_PENDING_CORE) $(P66_DIRECT_PENDING_TEST) -Wl,--gc-sections -o $@
test-p66-direct-pending: build/test-p66-direct-pending
	build/test-p66-direct-pending
sanitize-p66-direct-pending:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(P66_DIRECT_PENDING_FLAGS) $(P66_DIRECT_PENDING_CORE) $(P66_DIRECT_PENDING_TEST) -Wl,--gc-sections -o build/test-p66-direct-pending-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-p66-direct-pending-sanitize
c64-p66-direct-pending:
	$(MOS_CC) -Oz -fno-static-stack -mlto-zp=0 -DEXEC_NATIVE_FINISH_FAMILY=3 $(P66_DIRECT_PENDING_FLAGS) $(P66_DIRECT_PENDING_CORE) tests/p66_direct_pending_check.c tools/p66_direct_pending_diagnostic.c -Wl,-Map,build/p66_direct_pending.map -o build/p66_direct_pending.prg
test: test-p66-direct-pending
sanitize: sanitize-p66-direct-pending
.PHONY: all test sanitize test-p66-direct-pending sanitize-p66-direct-pending c64-p66-direct-pending
