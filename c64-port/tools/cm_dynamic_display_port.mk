# SPDX-License-Identifier: MIT
include tools/cm_dynamic_display.mk
all: build/test-cm-dynamic-display
build/test-cm-dynamic-display: $(CM_DDISPLAY_CORE) tests/test_cm_dynamic_display.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DDISPLAY_FLAGS) $^ -o $@
test: cm_dynamic_display-host
sanitize: cm_dynamic_display-sanitize
