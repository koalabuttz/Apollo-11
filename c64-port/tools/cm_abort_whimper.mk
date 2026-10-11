# SPDX-License-Identifier: MIT
CM_ABORT_WHIMPER_CORE = src/cm_abort_whimper.c src/cm_interbank_extended.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_abort_whimper_cases.c tests/cm_vac5_store_cases.c tests/cm_admission_complete_cases.c
build/cm-abort-whimper-native: $(CM_ABORT_WHIMPER_CORE) platform/host/cm_abort_whimper_snapshot.c include/cm_abort_whimper.h | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_ABORT_WHIMPER_CORE) platform/host/cm_abort_whimper_snapshot.c -o $@
cm-abort-whimper-differential: build/cm-abort-whimper-native prepare-cm-original-rope
	python3 tools/cm_abort_whimper_differential.py
.PHONY: test-cm-abort-whimper sanitize-cm-abort-whimper
test: test-cm-abort-whimper
test-cm-abort-whimper: cm-abort-whimper-differential
sanitize: sanitize-cm-abort-whimper
sanitize-cm-abort-whimper: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ABORT_WHIMPER_CORE) platform/host/cm_abort_whimper_snapshot.c -o build/cm-abort-whimper-sanitize
	python3 tools/cm_abort_whimper_sanitize.py
