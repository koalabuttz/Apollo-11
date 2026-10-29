# SPDX-License-Identifier: MIT
DSKY_LAMP_RELEASE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_LAMP_RELEASE_CORE = src/agc_word.c src/agc_multiword.c src/lm_interbank.c src/cm_interbank_complete.c src/executive.c src/p66_native.c src/lm_basic_wake.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/cm_basic_wake.c src/dsky_display_release.c src/dsky_lamp_release.c tests/p66_bias_cases.c tests/dsky_lamp_release_cases.c
DSKY_LAMP_RELEASE_TEST = tests/dsky_lamp_release_check.c tests/test_dsky_lamp_release.c
all: build/test-dsky-lamp-release
build/test-dsky-lamp-release: $(DSKY_LAMP_RELEASE_CORE) $(DSKY_LAMP_RELEASE_TEST) tests/dsky_lamp_release_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_LAMP_RELEASE_FLAGS) $(DSKY_LAMP_RELEASE_CORE) $(DSKY_LAMP_RELEASE_TEST) -o $@
test-dsky-lamp-release: build/test-dsky-lamp-release
	build/test-dsky-lamp-release
sanitize-dsky-lamp-release:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_LAMP_RELEASE_FLAGS) $(DSKY_LAMP_RELEASE_CORE) $(DSKY_LAMP_RELEASE_TEST) -o build/test-dsky-lamp-release-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-lamp-release-sanitize
c64-dsky-lamp-release:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_LAMP_RELEASE_TARGET_LM $(DSKY_LAMP_RELEASE_FLAGS) $(DSKY_LAMP_RELEASE_CORE) tests/dsky_lamp_release_check.c tools/dsky_lamp_release_diagnostic.c -Wl,-Map,build/dsky_lamp_release-lm.map -o build/dsky_lamp_release-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_LAMP_RELEASE_TARGET_CM $(DSKY_LAMP_RELEASE_FLAGS) $(DSKY_LAMP_RELEASE_CORE) tests/dsky_lamp_release_check.c tools/dsky_lamp_release_diagnostic.c -Wl,-Map,build/dsky_lamp_release-cm.map -o build/dsky_lamp_release-cm.prg
differential-dsky-lamp-release:
	python3 tools/dsky_lamp_release_differential.py
	python3 tools/freeze_dsky_lamp_release.py
test: test-dsky-lamp-release
sanitize: sanitize-dsky-lamp-release
.PHONY: all test sanitize test-dsky-lamp-release sanitize-dsky-lamp-release c64-dsky-lamp-release differential-dsky-lamp-release
