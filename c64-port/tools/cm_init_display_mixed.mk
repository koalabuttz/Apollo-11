# SPDX-License-Identifier: MIT
CM_INIT_DISPLAY_MIXED_CORE = src/cm_init_display_mixed.c src/cm_init_display.c src/cm_priority_complete.c src/cm_basic_priority.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/agc_word.c tests/cm_init_display_mixed_cases.c tests/cm_priority_complete_cases.c tests/cm_context_change_cases.c tests/cm_admission_complete_cases.c
build/cm-init-display-mixed-native: $(CM_INIT_DISPLAY_MIXED_CORE) platform/host/cm_init_display_mixed_snapshot.c include/cm_init_display_mixed.h | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_INIT_DISPLAY_MIXED_CORE) platform/host/cm_init_display_mixed_snapshot.c -o $@
cm-init-display-mixed-differential: build/cm-init-display-mixed-native prepare-cm-original-rope
	python3 tools/cm_init_display_mixed_differential.py
.PHONY: test-cm-init-display-mixed sanitize-cm-init-display-mixed
test: test-cm-init-display-mixed
test-cm-init-display-mixed: cm-init-display-mixed-differential
sanitize: sanitize-cm-init-display-mixed
sanitize-cm-init-display-mixed: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_INIT_DISPLAY_MIXED_CORE) platform/host/cm_init_display_mixed_snapshot.c -o build/cm-init-display-mixed-sanitize
	python3 tools/cm_init_display_mixed_sanitize.py
