# SPDX-License-Identifier: MIT
include tools/cm_minutes_seconds.mk
CM_MS_ROUTE_CORE = src/cm_minutes_seconds.c src/cm_decimal_pair.c src/cm_pinball.c src/dsky_pinball.c src/cm_interbank.c src/fixed_data.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/cm_minutes_seconds_cases.c
CM_DP_ROUTE_CORE = src/cm_decimal_pair.c src/cm_pinball.c src/dsky_pinball.c src/agc_word.c src/agc_multiword.c tests/cm_decimal_pair_cases.c
all: build/test-cm-minutes-seconds-port build/cm-minutes-seconds-route-port build/cm-decimal-pair-route-port
build/test-cm-minutes-seconds-port: tests/test_cm_minutes_seconds.c $(CM_MS_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/cm-minutes-seconds-route-port: tools/cm_minutes_seconds_snapshot.c $(CM_MS_ROUTE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/cm-decimal-pair-route-port: tools/cm_decimal_pair_snapshot.c $(CM_DP_ROUTE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-cm-minutes-seconds-port
test-cm-minutes-seconds-port: build/test-cm-minutes-seconds-port
	./build/test-cm-minutes-seconds-port
sanitize: sanitize-cm-minutes-seconds-port
sanitize-cm-minutes-seconds-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_cm_minutes_seconds.c $(CM_MS_CORE) -o build/test-cm-minutes-seconds-port-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-minutes-seconds-port-san
.PHONY: test-cm-minutes-seconds-port sanitize-cm-minutes-seconds-port
