# Real resident/basic composition; arithmetic coverage remains its own driver.
P66_BASIC_CORE = src/agc_word.c src/agc_multiword.c src/executive.c src/native_dispatch.c src/irq_delivery.c src/waitlist.c src/native_tasks.c src/phase.c src/p66_startup.c src/p66_native.c src/p66_mode_display.c src/dsky_relay.c src/dsky_display.c src/p66_basic.c
P66_BASIC_TEST = tests/p66_bias_cases.c tests/p66_mode_display_cases.c tests/p66_basic_check.c
P66_BASIC_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -DP66_BASIC_NO_LAUNCH
.PHONY: test-p66-basic sanitize-p66-basic c64-p66-basic
test-p66-basic:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_BASIC_FLAGS) $(P66_BASIC_CORE) $(P66_BASIC_TEST) tests/test_p66_basic.c -o build/test-p66-basic
	./build/test-p66-basic
sanitize-p66-basic:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(P66_BASIC_FLAGS) $(P66_BASIC_CORE) $(P66_BASIC_TEST) tests/test_p66_basic.c -o build/test-p66-basic-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-basic-san
c64-p66-basic:
	$(MOS_CC) $(CPPFLAGS) -Iplatform/c64 -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(P66_BASIC_FLAGS) -DP66_BASIC_C64_SCREEN $(P66_BASIC_CORE) $(P66_BASIC_TEST) platform/c64/dsky_display.c tools/p66_basic_diagnostic.c -Wl,-Map,build/p66_basic.map -o build/p66_basic.prg
test: test-p66-basic
sanitize: sanitize-p66-basic

P66_BASIC_LAUNCH_CORE = $(P66_BASIC_CORE) src/guidance_product.c src/p66_initialization.c src/p66_bias.c src/p66_major_mode.c src/p66_vertical_start.c src/p66_launch.c
.PHONY: test-p66-basic-launch sanitize-p66-basic-launch
test-p66-basic-launch:
	$(CC) $(CPPFLAGS) $(CFLAGS) -DP66_NATIVE_DISPATCH_ONLY $(P66_BASIC_LAUNCH_CORE) tests/p66_bias_cases.c tests/p66_vertical_start_cases.c tests/p66_launch_cases.c tests/test_p66_basic_launch.c -o build/test-p66-basic-launch
	./build/test-p66-basic-launch
sanitize-p66-basic-launch:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -DP66_NATIVE_DISPATCH_ONLY $(P66_BASIC_LAUNCH_CORE) tests/p66_bias_cases.c tests/p66_vertical_start_cases.c tests/p66_launch_cases.c tests/test_p66_basic_launch.c -o build/test-p66-basic-launch-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-basic-launch-san
test: test-p66-basic-launch
sanitize: sanitize-p66-basic-launch

P66_BASIC_TASK_TEST = tests/p66_basic_task_cases.c tests/p66_basic_task_check.c
.PHONY: test-p66-basic-task sanitize-p66-basic-task c64-p66-basic-task differential-p66-basic-task
test-p66-basic-task:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_BASIC_FLAGS) $(P66_BASIC_CORE) $(P66_BASIC_TASK_TEST) tests/test_p66_basic_task.c -o build/test-p66-basic-task
	./build/test-p66-basic-task
sanitize-p66-basic-task:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(P66_BASIC_FLAGS) $(P66_BASIC_CORE) $(P66_BASIC_TASK_TEST) tests/test_p66_basic_task.c -o build/test-p66-basic-task-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-basic-task-san
c64-p66-basic-task:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(P66_BASIC_FLAGS) -DP66_BASIC_TASK_C64 $(P66_BASIC_CORE) $(P66_BASIC_TASK_TEST) tools/p66_basic_task_diagnostic.c -Wl,-Map,build/p66_basic_task.map -o build/p66_basic_task.prg
differential-p66-basic-task:
	python3 tools/p66_basic_task_differential.py deps/fixture
	python3 tools/freeze_p66_basic_task.py
test: test-p66-basic-task
sanitize: sanitize-p66-basic-task
