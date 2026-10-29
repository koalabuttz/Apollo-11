# SPDX-License-Identifier: MIT
DSKY_LAMP_LIFECYCLE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_LAMP_LIFECYCLE_CORE = src/agc_word.c src/agc_multiword.c src/lm_interbank.c src/cm_interbank_complete.c src/dsky_source_alarm.c src/dsky_lamp_test.c src/executive.c src/p66_native.c src/lm_admission_image.c src/waitlist.c src/lm_waitlist_image.c src/lm_basic_wake.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/cm_basic_wake.c src/cm_admission_complete.c src/cm_waitlist_extended.c src/dsky_display_release.c src/dsky_lamp_release.c src/dsky_mode_admit.c src/dsky_lamp_lifecycle.c tests/p66_bias_cases.c tests/dsky_lamp_lifecycle_cases.c
DSKY_LAMP_LIFECYCLE_TEST = tests/dsky_lamp_lifecycle_check.c tests/test_dsky_lamp_lifecycle.c
all: build/test-dsky-lamp-lifecycle
build/test-dsky-lamp-lifecycle: $(DSKY_LAMP_LIFECYCLE_CORE) $(DSKY_LAMP_LIFECYCLE_TEST) tests/dsky_lamp_lifecycle_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_LAMP_LIFECYCLE_FLAGS) $(DSKY_LAMP_LIFECYCLE_CORE) $(DSKY_LAMP_LIFECYCLE_TEST) -o $@
test-dsky-lamp-lifecycle: build/test-dsky-lamp-lifecycle
	build/test-dsky-lamp-lifecycle
sanitize-dsky-lamp-lifecycle:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_LAMP_LIFECYCLE_FLAGS) $(DSKY_LAMP_LIFECYCLE_CORE) $(DSKY_LAMP_LIFECYCLE_TEST) -o build/test-dsky-lamp-lifecycle-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-lamp-lifecycle-sanitize
c64-dsky-lamp-lifecycle:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_LAMP_LIFECYCLE_TARGET_LM $(DSKY_LAMP_LIFECYCLE_FLAGS) $(DSKY_LAMP_LIFECYCLE_CORE) tests/dsky_lamp_lifecycle_check.c tools/dsky_lamp_lifecycle_diagnostic.c -Wl,-Map,build/dsky_lamp_lifecycle-lm.map -o build/dsky_lamp_lifecycle-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_LAMP_LIFECYCLE_TARGET_CM $(DSKY_LAMP_LIFECYCLE_FLAGS) $(DSKY_LAMP_LIFECYCLE_CORE) tests/dsky_lamp_lifecycle_check.c tools/dsky_lamp_lifecycle_diagnostic.c -Wl,-Map,build/dsky_lamp_lifecycle-cm.map -o build/dsky_lamp_lifecycle-cm.prg
differential-dsky-lamp-lifecycle:
	python3 tools/dsky_lamp_lifecycle_differential.py
	python3 tools/freeze_dsky_lamp_lifecycle.py
test: test-dsky-lamp-lifecycle
sanitize: sanitize-dsky-lamp-lifecycle
.PHONY: all test sanitize test-dsky-lamp-lifecycle sanitize-dsky-lamp-lifecycle c64-dsky-lamp-lifecycle differential-dsky-lamp-lifecycle
