# SPDX-License-Identifier: MIT
DSKY_OPERATOR_RELEASE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_OPERATOR_RELEASE_CORE = src/agc_word.c src/agc_multiword.c src/lm_interbank.c src/cm_interbank_complete.c src/executive.c src/p66_native.c src/lm_basic_wake.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/cm_basic_wake.c src/dsky_display_release.c src/dsky_lamp_release.c src/dsky_operator_release.c tests/p66_bias_cases.c tests/dsky_operator_release_cases.c
DSKY_OPERATOR_RELEASE_TEST = tests/dsky_operator_release_check.c tests/test_dsky_operator_release.c
all: build/test-dsky-operator-release
build/test-dsky-operator-release: $(DSKY_OPERATOR_RELEASE_CORE) $(DSKY_OPERATOR_RELEASE_TEST) tests/dsky_operator_release_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_OPERATOR_RELEASE_FLAGS) $(DSKY_OPERATOR_RELEASE_CORE) $(DSKY_OPERATOR_RELEASE_TEST) -o $@
test-dsky-operator-release: build/test-dsky-operator-release
	build/test-dsky-operator-release
sanitize-dsky-operator-release:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_OPERATOR_RELEASE_FLAGS) $(DSKY_OPERATOR_RELEASE_CORE) $(DSKY_OPERATOR_RELEASE_TEST) -o build/test-dsky-operator-release-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-operator-release-sanitize
c64-dsky-operator-release:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_OPERATOR_RELEASE_TARGET_LM $(DSKY_OPERATOR_RELEASE_FLAGS) $(DSKY_OPERATOR_RELEASE_CORE) tests/dsky_operator_release_check.c tools/dsky_operator_release_diagnostic.c -Wl,-Map,build/dsky_operator_release-lm.map -o build/dsky_operator_release-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_OPERATOR_RELEASE_TARGET_CM $(DSKY_OPERATOR_RELEASE_FLAGS) $(DSKY_OPERATOR_RELEASE_CORE) tests/dsky_operator_release_check.c tools/dsky_operator_release_diagnostic.c -Wl,-Map,build/dsky_operator_release-cm.map -o build/dsky_operator_release-cm.prg
differential-dsky-operator-release:
	python3 tools/dsky_operator_release_differential.py
	python3 tools/freeze_dsky_operator_release.py
test: test-dsky-operator-release
sanitize: sanitize-dsky-operator-release
.PHONY: all test sanitize test-dsky-operator-release sanitize-dsky-operator-release c64-dsky-operator-release differential-dsky-operator-release
