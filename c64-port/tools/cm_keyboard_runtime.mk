# SPDX-License-Identifier: MIT
CM_KEYBOARD_RUNTIME_FLAGS = -std=c99 -Wall -Wextra -Werror -ffunction-sections -Iinclude -Itests
CM_KEYBOARD_RUNTIME_CORE = src/agc_word.c src/agc_multiword.c src/cm_executive.c src/cm_dispatch.c src/cm_native_image.c src/cm_admission_image.c src/cm_keyboard_irq.c src/cm_idle_dispatch.c src/cm_basic_finish.c src/dsky_pinball.c src/dsky_nv_key.c src/dsky_charin_nv.c src/dsky_charin_number.c src/cm_charin_nv.c src/cm_charin_number.c src/cm_keyboard_runtime.c tests/p66_bias_cases.c tests/dsky_nv_key_cases.c tests/dsky_charin_nv_cases.c tests/dsky_charin_number_cases.c tests/cm_charin_nv_cases.c tests/cm_charin_number_cases.c tests/cm_keyboard_runtime_cases.c
CM_KEYBOARD_RUNTIME_TEST = tests/cm_keyboard_runtime_check.c tests/test_cm_keyboard_runtime.c
all: build/test-cm-keyboard-runtime
build/test-cm-keyboard-runtime: $(CM_KEYBOARD_RUNTIME_CORE) $(CM_KEYBOARD_RUNTIME_TEST) tests/cm_keyboard_runtime_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(CM_KEYBOARD_RUNTIME_FLAGS) $(CM_KEYBOARD_RUNTIME_CORE) $(CM_KEYBOARD_RUNTIME_TEST) -Wl,--gc-sections -o $@
test-cm-keyboard-runtime: build/test-cm-keyboard-runtime
	build/test-cm-keyboard-runtime
sanitize-cm-keyboard-runtime:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_KEYBOARD_RUNTIME_FLAGS) $(CM_KEYBOARD_RUNTIME_CORE) $(CM_KEYBOARD_RUNTIME_TEST) -Wl,--gc-sections -o build/test-cm-keyboard-runtime-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-keyboard-runtime-sanitize
c64-cm-keyboard-runtime:
	$(MOS_CC) -Iplatform/c64 -Oz -fno-static-stack -mlto-zp=0 $(CM_KEYBOARD_RUNTIME_FLAGS) $(CM_KEYBOARD_RUNTIME_CORE) src/dsky_keyboard.c platform/c64/dsky_keyboard.c tests/cm_keyboard_runtime_check.c tools/cm_keyboard_runtime_diagnostic.c -Wl,-Map,build/cm_keyboard_runtime.map -o build/cm_keyboard_runtime.prg
differential-cm-keyboard-runtime:
	python3 tools/cm_keyboard_runtime_differential.py
	python3 tools/freeze_cm_keyboard_runtime.py
test: test-cm-keyboard-runtime
sanitize: sanitize-cm-keyboard-runtime
.PHONY: all test sanitize test-cm-keyboard-runtime sanitize-cm-keyboard-runtime c64-cm-keyboard-runtime differential-cm-keyboard-runtime
