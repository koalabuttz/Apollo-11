# SPDX-License-Identifier: MIT
DSKY_LOAD_CONTROL_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_LOAD_CONTROL_CORE = src/agc_word.c src/agc_multiword.c src/executive.c src/lm_basic_wake.c src/cm_basic_wake.c src/cm_native_image.c src/cm_executive.c src/cm_dispatch.c src/dsky_display_release.c src/dsky_load_control.c tests/p66_bias_cases.c tests/dsky_load_control_cases.c
DSKY_LOAD_CONTROL_TEST = tests/dsky_load_control_check.c tests/test_dsky_load_control.c
all: build/test-dsky-load-control
build/test-dsky-load-control: $(DSKY_LOAD_CONTROL_CORE) $(DSKY_LOAD_CONTROL_TEST) tests/dsky_load_control_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_LOAD_CONTROL_FLAGS) $(DSKY_LOAD_CONTROL_CORE) $(DSKY_LOAD_CONTROL_TEST) -o $@
test-dsky-load-control: build/test-dsky-load-control
	build/test-dsky-load-control
sanitize-dsky-load-control:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_LOAD_CONTROL_FLAGS) $(DSKY_LOAD_CONTROL_CORE) $(DSKY_LOAD_CONTROL_TEST) -o build/test-dsky-load-control-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-load-control-sanitize
c64-dsky-load-control:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_LOAD_CONTROL_FLAGS) $(DSKY_LOAD_CONTROL_CORE) tests/dsky_load_control_check.c tools/dsky_load_control_diagnostic.c -Wl,-Map,build/dsky_load_control.map -o build/dsky_load_control.prg
differential-dsky-load-control:
	python3 tools/dsky_load_control_differential.py
	python3 tools/freeze_dsky_load_control.py
test: test-dsky-load-control
sanitize: sanitize-dsky-load-control
.PHONY: all test sanitize test-dsky-load-control sanitize-dsky-load-control c64-dsky-load-control differential-dsky-load-control
