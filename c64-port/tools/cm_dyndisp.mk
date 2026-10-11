# SPDX-License-Identifier: MIT
CM_DYN_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_DYN_CORE = src/cm_dyndisp.c src/agc_word.c src/agc_multiword.c src/guidance_product.c src/guidance_shift.c tests/cm_dyndisp_cases.c tests/matrix_cases.c tests/cm_dyndisp_check.c
cm_dyndisp-host:
	$(CC) -O2 $(CM_DYN_FLAGS) tests/test_cm_dyndisp.c $(CM_DYN_CORE) -o build/test-cm-dyndisp
	build/test-cm-dyndisp
cm_dyndisp-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(CM_DYN_FLAGS) tests/test_cm_dyndisp.c $(CM_DYN_CORE) -o build/test-cm-dyndisp-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-dyndisp-sanitize
	python3 tools/cm_dyndisp_sanitize_stream.py
cm_dyndisp-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_DYN_FLAGS) tools/cm_dyndisp_diagnostic.c $(CM_DYN_CORE) -Wl,-Map,build/cm_dyndisp.map -o build/cm_dyndisp.prg
cm_dyndisp-differential:
	python3 tools/cm_dyndisp_differential.py
.PHONY: cm_dyndisp-host cm_dyndisp-sanitize cm_dyndisp-c64 cm_dyndisp-differential
