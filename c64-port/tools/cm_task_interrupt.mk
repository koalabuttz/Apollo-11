# SPDX-License-Identifier: MIT
CM_TASK_INTERRUPT_CORE = src/cm_task_interrupt.c src/cm_native_image.c src/agc_word.c tests/cm_task_interrupt_cases.c
CM_TASK_INTERRUPT_CHECK = tests/cm_task_interrupt_check.c
CM_TASK_INTERRUPT_HEADERS = include/cm_task_interrupt.h include/cm_phase.h include/cm_native_image.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_task_interrupt_cases.h tests/cm_task_interrupt_check.h tests/cm_task_interrupt_vectors.h
.PHONY: cm-task-interrupt-differential test-cm-task-interrupt sanitize-cm-task-interrupt c64-cm-task-interrupt
all: build/cm-task-interrupt-native build/test-cm-task-interrupt
build/cm-task-interrupt-native: $(CM_TASK_INTERRUPT_CORE) platform/host/cm_task_interrupt_snapshot.c $(CM_TASK_INTERRUPT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TASK_INTERRUPT_CORE) platform/host/cm_task_interrupt_snapshot.c -o $@
cm-task-interrupt-differential: build/cm-task-interrupt-native
	python3 tools/cm_task_interrupt_differential.py
build/test-cm-task-interrupt: $(CM_TASK_INTERRUPT_CORE) $(CM_TASK_INTERRUPT_CHECK) tests/test_cm_task_interrupt.c $(CM_TASK_INTERRUPT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TASK_INTERRUPT_CORE) $(CM_TASK_INTERRUPT_CHECK) tests/test_cm_task_interrupt.c -o $@
test: test-cm-task-interrupt
test-cm-task-interrupt: build/test-cm-task-interrupt
	./build/test-cm-task-interrupt
sanitize: sanitize-cm-task-interrupt
sanitize-cm-task-interrupt: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_TASK_INTERRUPT_CORE) $(CM_TASK_INTERRUPT_CHECK) tests/test_cm_task_interrupt.c -o build/test-cm-task-interrupt-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-task-interrupt-sanitize
c64-cm-task-interrupt: build/cm-task-interrupt.prg
build/cm-task-interrupt.prg: $(CM_TASK_INTERRUPT_CORE) $(CM_TASK_INTERRUPT_CHECK) platform/c64/cm_task_interrupt_diagnostic.c $(CM_TASK_INTERRUPT_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-task-interrupt.map $(CM_TASK_INTERRUPT_CORE) $(CM_TASK_INTERRUPT_CHECK) platform/c64/cm_task_interrupt_diagnostic.c -o $@
