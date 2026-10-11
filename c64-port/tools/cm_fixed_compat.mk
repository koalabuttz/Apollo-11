# SPDX-License-Identifier: MIT
CM_FIXED_COMPAT_CORE = src/cm_fixed_compat.c src/cm_fixed_callers.c src/agc_word.c tests/cm_fixed_compat_cases.c
build/cm-fixed-compat-native: $(CM_FIXED_COMPAT_CORE) platform/host/cm_fixed_compat_snapshot.c include/cm_fixed_compat.h include/cm_fixed_callers.h src/cm_fixed_compat_words.inc | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_FIXED_COMPAT_CORE) platform/host/cm_fixed_compat_snapshot.c -o $@
cm-fixed-compat-differential: build/cm-fixed-compat-native
	python3 tools/cm_fixed_compat_differential.py
.PHONY: test-cm-fixed-compat sanitize-cm-fixed-compat
test: test-cm-fixed-compat
test-cm-fixed-compat: cm-fixed-compat-differential
sanitize: sanitize-cm-fixed-compat
sanitize-cm-fixed-compat: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_FIXED_COMPAT_CORE) platform/host/cm_fixed_compat_snapshot.c -o build/cm-fixed-compat-sanitize
	python3 tools/cm_fixed_compat_sanitize.py
