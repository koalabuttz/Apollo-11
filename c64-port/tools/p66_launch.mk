P66_LAUNCH_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/executive.c src/waitlist.c src/phase.c src/p66_startup.c src/p66_initialization.c src/p66_bias.c src/p66_major_mode.c src/p66_vertical_start.c src/p66_launch.c
P66_LAUNCH_TEST = tests/p66_bias_cases.c tests/p66_vertical_start_cases.c tests/p66_launch_cases.c tests/p66_launch_check.c
.PHONY: test-p66-launch sanitize-p66-launch c64-p66-launch differential-p66-launch
test-p66-launch:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(P66_LAUNCH_CORE) $(P66_LAUNCH_TEST) tests/test_p66_launch.c -o build/test-p66-launch
	./build/test-p66-launch
sanitize-p66-launch:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(P66_LAUNCH_CORE) $(P66_LAUNCH_TEST) tests/test_p66_launch.c -o build/test-p66-launch-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-p66-launch-san
c64-p66-launch:
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(P66_LAUNCH_CORE) $(P66_LAUNCH_TEST) tools/p66_launch_diagnostic.c -Wl,-Map,build/p66_launch.map -o build/p66_launch.prg
differential-p66-launch:
	python3 tools/p66_launch_differential.py deps/fixture
	python3 tools/freeze_p66_launch.py
test: test-p66-launch
sanitize: sanitize-p66-launch
