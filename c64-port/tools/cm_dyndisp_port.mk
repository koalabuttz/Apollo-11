# SPDX-License-Identifier: MIT
include tools/cm_dyndisp.mk
CM_DYNDISP_ROUTE_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/guidance_shift.c src/cm_dyndisp.c src/cm_dyndisp_route.c src/matrix_native.c src/gimbal_stage.c src/cm_powered_interface.c src/cm_powered_axis.c src/cm_powered_trig.c src/cm_rotation.c src/guidance_trig.c tests/matrix_cases.c tests/cm_dyndisp_cases.c tests/cm_dyndisp_apply.c
all: build/test-cm-dyndisp build/cm-dyndisp-route-port
build/test-cm-dyndisp: tests/test_cm_dyndisp.c $(CM_DYN_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/cm-dyndisp-route-port: tools/cm_dyndisp_snapshot.c $(CM_DYNDISP_ROUTE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: cm_dyndisp-host
sanitize: sanitize-cm-dyndisp-port
sanitize-cm-dyndisp-port: | build
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(CM_DYN_FLAGS) tests/test_cm_dyndisp.c $(CM_DYN_CORE) -o build/test-cm-dyndisp-port-san
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-dyndisp-port-san
.PHONY: sanitize-cm-dyndisp-port
