# SPDX-License-Identifier: MIT
DSKY_JAM_CONTROL_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_JAM_CONTROL_CORE = src/agc_word.c src/agc_multiword.c src/executive.c src/lm_basic_wake.c src/cm_basic_wake.c src/cm_native_image.c src/cm_executive.c src/cm_dispatch.c src/dsky_display_release.c src/dsky_load_control.c src/dsky_jam_control.c tests/p66_bias_cases.c tests/dsky_jam_control_cases.c
DSKY_JAM_CONTROL_TEST = tests/dsky_jam_control_check.c tests/test_dsky_jam_control.c
all: build/test-dsky-jam-control
build/test-dsky-jam-control: $(DSKY_JAM_CONTROL_CORE) $(DSKY_JAM_CONTROL_TEST) tests/dsky_jam_control_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_JAM_CONTROL_FLAGS) $(DSKY_JAM_CONTROL_CORE) $(DSKY_JAM_CONTROL_TEST) -o $@
test-dsky-jam-control: build/test-dsky-jam-control
	build/test-dsky-jam-control
sanitize-dsky-jam-control:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_JAM_CONTROL_FLAGS) $(DSKY_JAM_CONTROL_CORE) $(DSKY_JAM_CONTROL_TEST) -o build/test-dsky-jam-control-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-jam-control-sanitize
c64-dsky-jam-control:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_JAM_CONTROL_FLAGS) $(DSKY_JAM_CONTROL_CORE) tests/dsky_jam_control_check.c tools/dsky_jam_control_diagnostic.c -Wl,-Map,build/dsky_jam_control.map -o build/dsky_jam_control.prg
differential-dsky-jam-control:
	python3 tools/dsky_jam_control_differential.py
	python3 tools/freeze_dsky_jam_control.py
test: test-dsky-jam-control
sanitize: sanitize-dsky-jam-control
.PHONY: all test sanitize test-dsky-jam-control sanitize-dsky-jam-control c64-dsky-jam-control differential-dsky-jam-control
