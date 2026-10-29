# SPDX-License-Identifier: MIT
P66_DIRECT_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections
P66_DIRECT_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/executive.c src/waitlist.c src/phase.c src/p66_startup.c src/p66_initialization.c src/p66_bias.c src/p66_major_mode.c src/p66_vertical_start.c src/p66_launch.c src/p66_direct_launch.c src/p66_native.c src/native_dispatch.c src/irq_delivery.c tests/p66_bias_cases.c tests/p66_vertical_start_cases.c tests/p66_launch_cases.c tests/p66_direct_launch_cases.c
P66_DIRECT_TEST = tests/p66_direct_launch_check.c tests/test_p66_direct_launch.c
all: build/test-p66-direct-launch
build/test-p66-direct-launch: $(P66_DIRECT_CORE) $(P66_DIRECT_TEST) tests/p66_direct_launch_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(P66_DIRECT_FLAGS) $(P66_DIRECT_CORE) $(P66_DIRECT_TEST) -Wl,--gc-sections -o $@
test-p66-direct-launch: build/test-p66-direct-launch
	build/test-p66-direct-launch
sanitize-p66-direct-launch:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(P66_DIRECT_FLAGS) $(P66_DIRECT_CORE) $(P66_DIRECT_TEST) -Wl,--gc-sections -o build/test-p66-direct-launch-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-p66-direct-launch-sanitize
c64-p66-direct-launch:
	$(MOS_CC) -Oz -fno-static-stack -mlto-zp=0 -DEXEC_NATIVE_FINISH_FAMILY=3 $(P66_DIRECT_FLAGS) $(P66_DIRECT_CORE) tests/p66_direct_launch_check.c tools/p66_direct_launch_diagnostic.c -Wl,-Map,build/p66_direct_launch.map -o build/p66_direct_launch.prg
differential-p66-direct-launch:
	python3 tools/p66_direct_launch_differential.py deps/fixture
	python3 tools/freeze_p66_direct_launch.py
test: test-p66-direct-launch
sanitize: sanitize-p66-direct-launch
.PHONY: all test sanitize test-p66-direct-launch sanitize-p66-direct-launch c64-p66-direct-launch differential-p66-direct-launch
