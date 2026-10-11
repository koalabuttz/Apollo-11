# SPDX-License-Identifier: MIT
CM_TRANSFER_CORE = src/cm_task_transfer.c src/cm_waitlist.c src/cm_phase.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c tests/cm_task_transfer_cases.c
CM_TRANSFER_CHECK = tests/cm_task_transfer_check.c
CM_TRANSFER_HEADERS = include/cm_task_transfer.h include/cm_phase.h include/cm_native_image.h include/overlay_runtime.h include/agc_word.h include/agc_multiword.h tests/cm_task_transfer_cases.h tests/cm_task_transfer_check.h tests/cm_task_transfer_vectors.h
.PHONY: cm-task-transfer-differential test-cm-task-transfer sanitize-cm-task-transfer c64-cm-task-transfer
all: build/cm-task-transfer-native build/test-cm-task-transfer
build/cm-task-transfer-native: $(CM_TRANSFER_CORE) platform/host/cm_task_transfer_snapshot.c $(CM_TRANSFER_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TRANSFER_CORE) platform/host/cm_task_transfer_snapshot.c -o $@
cm-task-transfer-differential: build/cm-task-transfer-native
	python3 tools/cm_task_transfer_differential.py
build/test-cm-task-transfer: $(CM_TRANSFER_CORE) $(CM_TRANSFER_CHECK) tests/test_cm_task_transfer.c $(CM_TRANSFER_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TRANSFER_CORE) $(CM_TRANSFER_CHECK) tests/test_cm_task_transfer.c -o $@
test: test-cm-task-transfer
test-cm-task-transfer: build/test-cm-task-transfer
	./build/test-cm-task-transfer
sanitize: sanitize-cm-task-transfer
sanitize-cm-task-transfer: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_TRANSFER_CORE) $(CM_TRANSFER_CHECK) tests/test_cm_task_transfer.c -o build/test-cm-task-transfer-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-task-transfer-sanitize
c64-cm-task-transfer: build/cm-task-transfer.prg
build/cm-task-transfer.prg: $(CM_TRANSFER_CORE) $(CM_TRANSFER_CHECK) platform/c64/cm_task_transfer_diagnostic.c $(CM_TRANSFER_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-task-transfer.map $(CM_TRANSFER_CORE) $(CM_TRANSFER_CHECK) platform/c64/cm_task_transfer_diagnostic.c -o $@
