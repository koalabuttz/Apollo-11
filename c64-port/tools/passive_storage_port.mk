# SPDX-License-Identifier: MIT
include tools/passive_storage.mk
all: build/passive-storage-lm-host build/passive-storage-cm-host
build/passive-storage-lm-host: tests/passive_storage_host.c src/lm_fixed_compat.c src/lm_fixed_compat_words.inc | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DPASSIVE_STORAGE_LM tests/passive_storage_host.c src/lm_fixed_compat.c -o $@
build/passive-storage-cm-host: tests/passive_storage_host.c src/cm_fixed_compat.c src/cm_fixed_compat_words.inc | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/passive_storage_host.c src/cm_fixed_compat.c -o $@
test: test-passive-storage
test-passive-storage: | build
	python3 tools/passive_storage_differential.py
sanitize: test-passive-storage
.PHONY: test-passive-storage
