# SPDX-License-Identifier: MIT
CM_SUPER_DISPATCH_CORE = src/cm_super_dispatch.c src/agc_word.c tests/cm_super_dispatch_cases.c
build/cm-super-dispatch-native: $(CM_SUPER_DISPATCH_CORE) platform/host/cm_super_dispatch_snapshot.c include/cm_super_dispatch.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_SUPER_DISPATCH_CORE) platform/host/cm_super_dispatch_snapshot.c -o $@
.PHONY: test-cm-super-dispatch sanitize-cm-super-dispatch cm-super-dispatch-differential
test: test-cm-super-dispatch
test-cm-super-dispatch: cm-super-dispatch-differential
cm-super-dispatch-differential: build/cm-super-dispatch-native prepare-cm-original-rope
	python3 tools/cm_super_dispatch_differential.py
sanitize: sanitize-cm-super-dispatch
sanitize-cm-super-dispatch: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_SUPER_DISPATCH_CORE) platform/host/cm_super_dispatch_snapshot.c -o build/cm-super-dispatch-sanitize
	python3 tools/cm_super_dispatch_sanitize.py
