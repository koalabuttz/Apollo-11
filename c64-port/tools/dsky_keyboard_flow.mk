DSKY_KEYBOARD_FLOW_CORE = src/agc_word.c src/agc_multiword.c src/executive.c src/native_dispatch.c src/irq_delivery.c src/waitlist.c src/native_tasks.c src/phase.c src/p66_startup.c src/p66_native.c src/p66_basic.c src/p66_mode_display.c src/dsky_relay.c src/dsky_display.c src/dsky_pinball.c src/dsky_nv_key.c src/dsky_charin_nv.c src/dsky_keycom.c src/dsky_keyrupt.c src/dsky_key_irq.c src/dsky_keyboard.c
DSKY_KEYBOARD_FLOW_TEST = tests/p66_bias_cases.c tests/dsky_nv_key_cases.c tests/dsky_keyboard_flow_cases.c tests/dsky_keyboard_flow_check.c
DSKY_KEYBOARD_FLOW_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -DP66_BASIC_NO_LAUNCH
.PHONY: all test sanitize test-dsky-keyboard-flow sanitize-dsky-keyboard-flow c64-dsky-keyboard-flow differential-dsky-keyboard-flow
build/test-dsky-keyboard-flow: $(DSKY_KEYBOARD_FLOW_CORE) $(DSKY_KEYBOARD_FLOW_TEST) tests/test_dsky_keyboard_flow.c tests/dsky_keyboard_flow_vectors.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_KEYBOARD_FLOW_FLAGS) $(DSKY_KEYBOARD_FLOW_CORE) $(DSKY_KEYBOARD_FLOW_TEST) tests/test_dsky_keyboard_flow.c -o build/test-dsky-keyboard-flow
test-dsky-keyboard-flow: build/test-dsky-keyboard-flow
	./build/test-dsky-keyboard-flow
sanitize-dsky-keyboard-flow:
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(DSKY_KEYBOARD_FLOW_FLAGS) $(DSKY_KEYBOARD_FLOW_CORE) $(DSKY_KEYBOARD_FLOW_TEST) tests/test_dsky_keyboard_flow.c -o build/test-dsky-keyboard-flow-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-keyboard-flow-san
c64-dsky-keyboard-flow:
	$(MOS_CC) $(CPPFLAGS) -DEXEC_NATIVE_FINISH_FAMILY=2 -Iplatform/c64 -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(DSKY_KEYBOARD_FLOW_FLAGS) -DDSKY_KEYBOARD_FLOW_C64 $(DSKY_KEYBOARD_FLOW_CORE) $(DSKY_KEYBOARD_FLOW_TEST) platform/c64/dsky_keyboard.c tools/dsky_keyboard_flow_diagnostic.c -Wl,-Map,build/dsky_keyboard_flow.map -o build/dsky_keyboard_flow.prg
differential-dsky-keyboard-flow:
	python3 tools/dsky_keyboard_flow_differential.py deps/fixture
	python3 tools/freeze_dsky_keyboard_flow.py
test: test-dsky-keyboard-flow
sanitize: sanitize-dsky-keyboard-flow
all: build/test-dsky-keyboard-flow
