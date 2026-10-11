# SPDX-License-Identifier: MIT
include tools/cm_display_control.mk
test: cm_display_control-host
sanitize: sanitize-cm-display-control-frozen
sanitize-cm-display-control-frozen: | build
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMDCTRL_FLAGS) tests/test_cm_display_control.c tests/cm_display_control_check.c $(CMDCTRL_CORE) -o build/test-cm-display-control-frozen-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-display-control-frozen-sanitize
.PHONY: sanitize-cm-display-control-frozen
