# SPDX-License-Identifier: MIT
CM_CHARIN_NV_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
CM_CHARIN_NV_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_nv_key.c src/dsky_charin_nv.c src/cm_charin_nv.c tests/p66_bias_cases.c tests/dsky_nv_key_cases.c tests/dsky_charin_nv_cases.c tests/cm_charin_nv_cases.c
CM_CHARIN_NV_TEST = tests/test_cm_charin_nv.c tests/cm_charin_nv_check.c
all: build/test-cm-charin-nv
build/test-cm-charin-nv: $(CM_CHARIN_NV_CORE) $(CM_CHARIN_NV_TEST) tests/cm_charin_nv_vectors.h
	@mkdir -p build
	$(CC) -O2 -Wconversion -pedantic $(CM_CHARIN_NV_FLAGS) $(CM_CHARIN_NV_CORE) $(CM_CHARIN_NV_TEST) -o $@
test-cm-charin-nv: build/test-cm-charin-nv
	build/test-cm-charin-nv
sanitize-cm-charin-nv:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_CHARIN_NV_FLAGS) $(CM_CHARIN_NV_CORE) $(CM_CHARIN_NV_TEST) -o build/test-cm-charin-nv-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-charin-nv-sanitize
c64-cm-charin-nv:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_CHARIN_NV_FLAGS) $(CM_CHARIN_NV_CORE) tests/cm_charin_nv_check.c tools/cm_charin_nv_diagnostic.c -Wl,-Map,build/cm_charin_nv.map -o build/cm_charin_nv.prg
differential-cm-charin-nv:
	python3 tools/cm_charin_nv_differential.py
	python3 tools/freeze_cm_charin_nv.py
test: test-cm-charin-nv
sanitize: sanitize-cm-charin-nv
.PHONY: all test sanitize test-cm-charin-nv sanitize-cm-charin-nv c64-cm-charin-nv differential-cm-charin-nv
