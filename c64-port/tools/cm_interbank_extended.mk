# SPDX-License-Identifier: MIT
CM_INTERBANK_EXTENDED_CORE = src/cm_interbank_extended.c src/cm_fixed_compat.c src/agc_word.c tests/cm_interbank_extended_cases.c tests/cm_fixed_compat_cases.c
build/cm-interbank-extended-native: $(CM_INTERBANK_EXTENDED_CORE) platform/host/cm_interbank_extended_snapshot.c include/cm_interbank_extended.h src/cm_fixed_compat_words.inc | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_INTERBANK_EXTENDED_CORE) platform/host/cm_interbank_extended_snapshot.c -o $@
cm-interbank-extended-differential: build/cm-interbank-extended-native
	python3 tools/cm_interbank_extended_differential.py
.PHONY: test-cm-interbank-extended sanitize-cm-interbank-extended
test: test-cm-interbank-extended
test-cm-interbank-extended: cm-interbank-extended-differential
sanitize: sanitize-cm-interbank-extended
sanitize-cm-interbank-extended: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_INTERBANK_EXTENDED_CORE) platform/host/cm_interbank_extended_snapshot.c -o build/cm-interbank-extended-sanitize
	python3 tools/cm_interbank_extended_sanitize.py
