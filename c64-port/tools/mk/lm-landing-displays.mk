# SPDX-License-Identifier: MIT
# Milestone lm-landing-displays (lane F): Luminary099 LANDING_ANALOG_DISPLAYS as native
# routines of the control-transfer ABI.
LM_LAD_SRC = src/lm_landing_displays.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_landing_displays_cases.c
LM_LAD_HDR = include/lm_landing_displays.h include/lm_flight_io.h include/agc_image_ops.h include/agc_transfer.h src/lm_landing_displays_sites.inc tests/lm_landing_displays_cases.h
LM_LAD_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-landing-displays sanitize-lm-landing-displays c64-lm-landing-displays vice-lm-landing-displays lm-landing-displays-differential
all: build/test-lm-landing-displays
test: test-lm-landing-displays
sanitize: sanitize-lm-landing-displays
build/test-lm-landing-displays: $(LM_LAD_SRC) tests/test_lm_landing_displays.c tests/lm_landing_displays_vectors.h $(LM_LAD_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_LAD_FLAGS) -DLM_LAD_COUNT_PATHS $(LM_LAD_SRC) tests/test_lm_landing_displays.c -o $@
test-lm-landing-displays: build/test-lm-landing-displays
	./build/test-lm-landing-displays
	python3 tools/lm_landing_displays_source.py
sanitize-lm-landing-displays: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_LAD_FLAGS) -DLM_LAD_COUNT_PATHS -fsanitize=address,undefined $(LM_LAD_SRC) tests/test_lm_landing_displays.c -o build/test-lm-landing-displays-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-landing-displays-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-landing-displays-native: $(LM_LAD_SRC) platform/host/lm_landing_displays_snapshot.c $(LM_LAD_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_LAD_FLAGS) $(LM_LAD_SRC) platform/host/lm_landing_displays_snapshot.c -o $@
lm-landing-displays-differential: build/lm-landing-displays-native
	python3 tools/lm_landing_displays_freeze.py
	git diff --exit-code -- tests/lm_landing_displays_vectors.h tests/lm_landing_displays_c64_vectors.h
c64-lm-landing-displays: build/lm-landing-displays.prg
build/lm-landing-displays.prg: $(LM_LAD_SRC) platform/c64/lm_landing_displays_diagnostic.c tests/lm_landing_displays_c64_vectors.h $(LM_LAD_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -DP66_NATIVE_DISPATCH_ONLY -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-landing-displays.map $(LM_LAD_SRC) platform/c64/lm_landing_displays_diagnostic.c -o $@
vice-lm-landing-displays: build/lm-landing-displays.prg
	python3 tools/vice_run.py build/lm-landing-displays.prg --cycles 3000000000 --timeout 1200
