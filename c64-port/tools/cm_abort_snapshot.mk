# SPDX-License-Identifier: MIT
CM_ABORT_SNAPSHOT_CORE = src/cm_abort_snapshot.c src/cm_alarm_record.c src/cm_vac5_store.c src/cm_interbank_extended.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_abort_snapshot_cases.c tests/cm_vac5_store_cases.c tests/cm_admission_complete_cases.c
build/cm-abort-snapshot-native: $(CM_ABORT_SNAPSHOT_CORE) platform/host/cm_abort_snapshot_snapshot.c include/cm_abort_snapshot.h | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_ABORT_SNAPSHOT_CORE) platform/host/cm_abort_snapshot_snapshot.c -o $@
cm-abort-snapshot-differential: build/cm-abort-snapshot-native prepare-cm-original-rope
	python3 tools/cm_abort_snapshot_differential.py
.PHONY: test-cm-abort-snapshot sanitize-cm-abort-snapshot
test: test-cm-abort-snapshot
test-cm-abort-snapshot: cm-abort-snapshot-differential
sanitize: sanitize-cm-abort-snapshot
sanitize-cm-abort-snapshot: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ABORT_SNAPSHOT_CORE) platform/host/cm_abort_snapshot_snapshot.c -o build/cm-abort-snapshot-sanitize
	python3 tools/cm_abort_snapshot_sanitize.py
