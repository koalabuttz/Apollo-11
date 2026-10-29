# SPDX-License-Identifier: MIT
CM_IDLE_WAKE_CORE = src/cm_idle_wake.c src/cm_idle_interrupt.c src/cm_basic_sleep.c src/cm_wake_complete.c src/cm_task_transfer.c src/cm_idle_dispatch.c src/service_routines.c src/agc_word.c src/cm_native_image.c src/cm_dispatch.c src/cm_executive.c src/cm_waitlist.c tests/cm_idle_wake_cases.c tests/cm_idle_interrupt_cases.c
CM_IDLE_WAKE_HEADERS = include/cm_idle_wake.h include/cm_idle_interrupt.h include/cm_basic_sleep.h include/cm_wake_complete.h include/cm_task_transfer.h include/cm_idle_dispatch.h include/service_routines.h include/cm_native_image.h include/cm_dispatch.h include/cm_executive.h include/cm_waitlist.h include/agc_word.h include/overlay_runtime.h tests/cm_idle_wake_cases.h tests/cm_idle_interrupt_cases.h tools/cm_idle_interrupt_dump.h tools/cm_alarm2_image_dump.h
build/cm-idle-wake-native: $(CM_IDLE_WAKE_CORE) platform/host/cm_idle_wake_snapshot.c $(CM_IDLE_WAKE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IDLE_WAKE_CORE) platform/host/cm_idle_wake_snapshot.c -o $@
.PHONY: test-cm-idle-wake sanitize-cm-idle-wake cm-idle-wake-differential
test: test-cm-idle-wake
test-cm-idle-wake: cm-idle-wake-differential
cm-idle-wake-differential: build/cm-idle-wake-native prepare-cm-original-rope
	python3 tools/cm_idle_wake_differential.py
sanitize: sanitize-cm-idle-wake
sanitize-cm-idle-wake: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined $(CM_IDLE_WAKE_CORE) platform/host/cm_idle_wake_snapshot.c -o build/cm-idle-wake-sanitize
	python3 tools/cm_idle_wake_sanitize.py
