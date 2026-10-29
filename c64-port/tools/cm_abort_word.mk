# SPDX-License-Identifier: MIT
CM_ABORT_WORD_CORE = src/cm_abort_word.c src/cm_fixed_compat.c src/cm_native_image.c src/agc_word.c tests/cm_abort_word_cases.c tests/cm_admission_complete_cases.c
build/cm-abort-word-native: $(CM_ABORT_WORD_CORE) platform/host/cm_abort_word_snapshot.c include/cm_abort_word.h include/cm_fixed_compat.h src/cm_fixed_compat_words.inc | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_ABORT_WORD_CORE) platform/host/cm_abort_word_snapshot.c -o $@
cm-abort-word-differential: build/cm-abort-word-native prepare-cm-original-rope
	python3 tools/cm_abort_word_differential.py
.PHONY: cm-abort-word-differential test-cm-abort-word sanitize-cm-abort-word
test: test-cm-abort-word
test-cm-abort-word: cm-abort-word-differential
sanitize: sanitize-cm-abort-word
sanitize-cm-abort-word: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_ABORT_WORD_CORE) platform/host/cm_abort_word_snapshot.c -o build/cm-abort-word-sanitize
	python3 tools/cm_abort_word_sanitize.py
