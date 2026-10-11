# SPDX-License-Identifier: MIT
CM_VAC5_STORE_CORE = src/cm_vac5_store.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_vac5_store_cases.c tests/cm_admission_complete_cases.c
build/cm-vac5-store-native: $(CM_VAC5_STORE_CORE) platform/host/cm_vac5_store_snapshot.c include/cm_vac5_store.h | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_VAC5_STORE_CORE) platform/host/cm_vac5_store_snapshot.c -o $@
cm-vac5-store-differential: build/cm-vac5-store-native prepare-cm-original-rope
	python3 tools/cm_vac5_store_differential.py
.PHONY: test-cm-vac5-store sanitize-cm-vac5-store
test: test-cm-vac5-store
test-cm-vac5-store: cm-vac5-store-differential
sanitize: sanitize-cm-vac5-store
sanitize-cm-vac5-store: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_VAC5_STORE_CORE) platform/host/cm_vac5_store_snapshot.c -o build/cm-vac5-store-sanitize
	python3 tools/cm_vac5_store_sanitize.py
