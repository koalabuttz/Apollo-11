# SPDX-License-Identifier: MIT
CM_KEYBOARD_IRQ_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
CM_KEYBOARD_IRQ_CORE = src/agc_word.c src/cm_executive.c src/cm_native_image.c src/cm_admission_image.c src/cm_keyboard_irq.c tests/cm_keyboard_irq_cases.c
CM_KEYBOARD_IRQ_TEST = tests/test_cm_keyboard_irq.c tests/cm_keyboard_irq_check.c
all: build/test-cm-keyboard-irq
build/test-cm-keyboard-irq: $(CM_KEYBOARD_IRQ_CORE) $(CM_KEYBOARD_IRQ_TEST) tests/cm_keyboard_irq_vectors.h
	@mkdir -p build
	$(CC) -O2 -Wconversion -pedantic $(CM_KEYBOARD_IRQ_FLAGS) $(CM_KEYBOARD_IRQ_CORE) $(CM_KEYBOARD_IRQ_TEST) -o $@
test-cm-keyboard-irq: build/test-cm-keyboard-irq
	build/test-cm-keyboard-irq
sanitize-cm-keyboard-irq:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_KEYBOARD_IRQ_FLAGS) $(CM_KEYBOARD_IRQ_CORE) $(CM_KEYBOARD_IRQ_TEST) -o build/test-cm-keyboard-irq-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-keyboard-irq-sanitize
c64-cm-keyboard-irq:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_KEYBOARD_IRQ_FLAGS) $(CM_KEYBOARD_IRQ_CORE) tests/cm_keyboard_irq_check.c tools/cm_keyboard_irq_diagnostic.c -Wl,-Map,build/cm_keyboard_irq.map -o build/cm_keyboard_irq.prg
differential-cm-keyboard-irq:
	python3 tools/cm_keyboard_irq_differential.py
	python3 tools/freeze_cm_keyboard_irq.py
test: test-cm-keyboard-irq
sanitize: sanitize-cm-keyboard-irq
.PHONY: all test sanitize test-cm-keyboard-irq sanitize-cm-keyboard-irq c64-cm-keyboard-irq differential-cm-keyboard-irq
