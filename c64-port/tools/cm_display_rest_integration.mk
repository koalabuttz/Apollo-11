# SPDX-License-Identifier: MIT
CM_DISPLAY_REST_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_DISPLAY_REST_CORE = src/agc_word.c src/cm_display_rest.c tests/cm_display_rest_cases.c tests/cm_display_rest_check.c
all: build/test-cm-display-rest build/cm-display-rest-route-port
build/test-cm-display-rest: tests/test_cm_display_rest.c $(CM_DISPLAY_REST_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DISPLAY_REST_FLAGS) $^ -o $@
build/cm-display-rest-route-port: tools/cm_display_rest_snapshot.c $(CM_DISPLAY_REST_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_DISPLAY_REST_FLAGS) $^ -o $@

test: test-cm-display-rest-integrated
test-cm-display-rest-integrated: build/test-cm-display-rest build/cm-display-rest-route-port
	./build/test-cm-display-rest
	python3 tools/qualified_stream_host.py --archive evidence/cm-display-rest-reference.snapshots.xz --sha256 b50aeba703c0b9e1cf5b04bf8c9ee8099c38b2663e0026e8e9b86a40dc7e315f --binary build/cm-display-rest-route-port
sanitize: sanitize-cm-display-rest-integrated
sanitize-cm-display-rest-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_cm_display_rest.c src/agc_word.c src/cm_display_rest.c tests/cm_display_rest_cases.c tests/cm_display_rest_check.c -o build/test-cm-display-rest-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-display-rest-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/cm_display_rest_snapshot.c src/agc_word.c src/cm_display_rest.c tests/cm_display_rest_cases.c tests/cm_display_rest_check.c -o build/cm-display-rest-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/cm-display-rest-reference.snapshots.xz --sha256 b50aeba703c0b9e1cf5b04bf8c9ee8099c38b2663e0026e8e9b86a40dc7e315f --binary build/cm-display-rest-integrated-stream-san
.PHONY: test-cm-display-rest-integrated sanitize-cm-display-rest-integrated
