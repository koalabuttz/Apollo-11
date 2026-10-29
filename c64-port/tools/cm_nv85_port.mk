# SPDX-License-Identifier: MIT
include tools/cm_nv85.mk
CM_NV85_ROUTE_CORE = src/cm_nv85.c src/cm_nv85_route.c src/cm_formatter.c src/cm_pinball.c src/dsky_two_digit.c src/dsky_decimal.c src/dsky_pinball.c src/cm_scale.c src/dsky_scale.c src/guidance_product.c src/agc_word.c src/agc_multiword.c src/cm_clockjob.c src/cm_display_control.c src/cm_makeplay.c src/cm_basic_priority.c src/cm_basic_wake.c src/cm_basic_finish.c src/cm_native_image.c src/cm_executive.c src/cm_dispatch.c tests/cm_trig_cases.c tests/cm_clockjob_cases.c tests/cm_nv85_cases.c tests/cm_nv85_apply.c
all: build/test-cm-nv85 build/cm-nv85-route-port
build/test-cm-nv85: tests/test_cm_nv85.c $(CM_NV85_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_NV85_FLAGS) $^ -o $@
build/cm-nv85-route-port: tools/cm_nv85_snapshot.c $(CM_NV85_ROUTE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_NV85_FLAGS) $^ -o $@
test: cm_nv85-host
sanitize: sanitize-cm-nv85-port
sanitize-cm-nv85-port: | build
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(CM_NV85_FLAGS) tests/test_cm_nv85.c $(CM_NV85_CORE) -o build/test-cm-nv85-port-san
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-nv85-port-san
.PHONY: sanitize-cm-nv85-port
