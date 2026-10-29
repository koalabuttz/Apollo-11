# SPDX-License-Identifier: MIT
P66_STATUS_CORE = src/agc_word.c src/executive.c src/native_dispatch.c src/irq_delivery.c src/p66_native.c tests/p66_status_check.c
P66_STATUS_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests -DP66_NATIVE_DISPATCH_ONLY
.PHONY: test-p66-status sanitize-p66-status c64-p66-status
test-p66-status: | build
	$(CC) $(P66_STATUS_FLAGS) -O2 -Wconversion -pedantic $(P66_STATUS_CORE) tests/test_p66_status.c -o build/test-p66-status
	build/test-p66-status
sanitize-p66-status: | build
	$(CC) $(P66_STATUS_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(P66_STATUS_CORE) tests/test_p66_status.c -o build/test-p66-status-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-p66-status-sanitize
c64-p66-status: | build
	$(MOS_CC) $(P66_STATUS_FLAGS) -Oz -fno-static-stack -mlto-zp=0 -T platform/c64/overlay_resident.ld -Wl,-Map,build/p66-status.map $(P66_STATUS_CORE) platform/c64/p66_status_diagnostic.c -o build/p66-status.prg

test: test-p66-status
sanitize: sanitize-p66-status
