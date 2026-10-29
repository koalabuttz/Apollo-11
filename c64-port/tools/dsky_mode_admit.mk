# SPDX-License-Identifier: MIT
DSKY_MODE_ADMIT_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_MODE_ADMIT_CORE = src/agc_word.c src/executive.c src/p66_native.c src/lm_admission_image.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/cm_admission_complete.c src/dsky_mode_admit.c tests/p66_bias_cases.c tests/dsky_mode_admit_cases.c
DSKY_MODE_ADMIT_TEST = tests/dsky_mode_admit_check.c tests/test_dsky_mode_admit.c
all: build/test-dsky-mode-admit
build/test-dsky-mode-admit: $(DSKY_MODE_ADMIT_CORE) $(DSKY_MODE_ADMIT_TEST) tests/dsky_mode_admit_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_MODE_ADMIT_FLAGS) $(DSKY_MODE_ADMIT_CORE) $(DSKY_MODE_ADMIT_TEST) -o $@
test-dsky-mode-admit: build/test-dsky-mode-admit
	build/test-dsky-mode-admit
sanitize-dsky-mode-admit:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_MODE_ADMIT_FLAGS) $(DSKY_MODE_ADMIT_CORE) $(DSKY_MODE_ADMIT_TEST) -o build/test-dsky-mode-admit-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-mode-admit-sanitize
c64-dsky-mode-admit:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_MODE_ADMIT_TARGET_LM $(DSKY_MODE_ADMIT_FLAGS) $(DSKY_MODE_ADMIT_CORE) tests/dsky_mode_admit_check.c tools/dsky_mode_admit_diagnostic.c -Wl,-Map,build/dsky_mode_admit-lm.map -o build/dsky_mode_admit-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_MODE_ADMIT_TARGET_CM $(DSKY_MODE_ADMIT_FLAGS) $(DSKY_MODE_ADMIT_CORE) tests/dsky_mode_admit_check.c tools/dsky_mode_admit_diagnostic.c -Wl,-Map,build/dsky_mode_admit-cm.map -o build/dsky_mode_admit-cm.prg
differential-dsky-mode-admit:
	python3 tools/dsky_mode_admit_differential.py
	python3 tools/freeze_dsky_mode_admit.py
test: test-dsky-mode-admit
sanitize: sanitize-dsky-mode-admit
.PHONY: all test sanitize test-dsky-mode-admit sanitize-dsky-mode-admit c64-dsky-mode-admit differential-dsky-mode-admit
