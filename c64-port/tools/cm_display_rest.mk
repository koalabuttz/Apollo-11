# SPDX-License-Identifier: MIT
CM_DISPLAY_REST_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_DISPLAY_REST_CORE = src/agc_word.c src/cm_display_rest.c tests/cm_display_rest_cases.c tests/cm_display_rest_check.c
all: build/test-cm-display-rest build/cm-display-rest-route-port
build/test-cm-display-rest: tests/test_cm_display_rest.c $(CM_DISPLAY_REST_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DISPLAY_REST_FLAGS) $^ -o $@
build/cm-display-rest-route-port: tools/cm_display_rest_snapshot.c $(CM_DISPLAY_REST_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DISPLAY_REST_FLAGS) $^ -o $@
cm-display-rest-host: build/test-cm-display-rest
	build/test-cm-display-rest
cm-display-rest-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(CM_DISPLAY_REST_FLAGS) tests/test_cm_display_rest.c $(CM_DISPLAY_REST_CORE) -o build/test-cm-display-rest-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-display-rest-sanitize
	python3 tools/cm_display_rest_sanitize.py
cm-display-rest-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_DISPLAY_REST_FLAGS) tools/cm_display_rest_diagnostic.c $(CM_DISPLAY_REST_CORE) -Wl,-Map,build/cm_display_rest.map -o build/cm_display_rest.prg
test: cm-display-rest-host
sanitize: cm-display-rest-sanitize
.PHONY: cm-display-rest-host cm-display-rest-sanitize cm-display-rest-c64
