# SPDX-License-Identifier: MIT
CM_IDLE_INTERRUPT_CORE = src/cm_idle_interrupt.c src/agc_word.c tests/cm_idle_interrupt_cases.c
build/cm-idle-interrupt-native: $(CM_IDLE_INTERRUPT_CORE) platform/host/cm_idle_interrupt_snapshot.c include/cm_idle_interrupt.h tests/cm_idle_interrupt_cases.h tools/cm_idle_interrupt_dump.h tools/cm_alarm2_image_dump.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IDLE_INTERRUPT_CORE) platform/host/cm_idle_interrupt_snapshot.c -o $@
.PHONY: test-cm-idle-interrupt sanitize-cm-idle-interrupt cm-idle-interrupt-differential
test: test-cm-idle-interrupt
build/test-cm-idle-interrupt: $(CM_IDLE_INTERRUPT_CORE) tests/test_cm_idle_interrupt.c include/cm_idle_interrupt.h tests/cm_idle_interrupt_cases.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IDLE_INTERRUPT_CORE) tests/test_cm_idle_interrupt.c -o $@
test-cm-idle-interrupt: cm-idle-interrupt-differential build/test-cm-idle-interrupt
	./build/test-cm-idle-interrupt
cm-idle-interrupt-differential: build/cm-idle-interrupt-native prepare-cm-original-rope
	python3 tools/cm_idle_interrupt_differential.py
sanitize: sanitize-cm-idle-interrupt
sanitize-cm-idle-interrupt: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(CM_IDLE_INTERRUPT_CORE) platform/host/cm_idle_interrupt_snapshot.c -o build/cm-idle-interrupt-sanitize
	python3 tools/cm_idle_interrupt_sanitize.py
