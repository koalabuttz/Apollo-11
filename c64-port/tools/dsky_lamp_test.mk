# SPDX-License-Identifier: MIT
DSKY_LAMP_TEST_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_LAMP_TEST_CORE = src/agc_word.c src/lm_interbank.c src/cm_interbank_complete.c src/dsky_source_alarm.c src/dsky_lamp_test.c tests/p66_bias_cases.c tests/dsky_lamp_test_cases.c
DSKY_LAMP_TEST_TEST = tests/dsky_lamp_test_check.c tests/test_dsky_lamp_test.c
all: build/test-dsky-lamp-test
build/test-dsky-lamp-test: $(DSKY_LAMP_TEST_CORE) $(DSKY_LAMP_TEST_TEST) tests/dsky_lamp_test_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_LAMP_TEST_FLAGS) $(DSKY_LAMP_TEST_CORE) $(DSKY_LAMP_TEST_TEST) -o $@
test-dsky-lamp-test: build/test-dsky-lamp-test
	build/test-dsky-lamp-test
sanitize-dsky-lamp-test:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_LAMP_TEST_FLAGS) $(DSKY_LAMP_TEST_CORE) $(DSKY_LAMP_TEST_TEST) -o build/test-dsky-lamp-test-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-lamp-test-sanitize
c64-dsky-lamp-test:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_LAMP_TEST_TARGET_LM $(DSKY_LAMP_TEST_FLAGS) $(DSKY_LAMP_TEST_CORE) tests/dsky_lamp_test_check.c tools/dsky_lamp_test_diagnostic.c -Wl,-Map,build/dsky_lamp_test-lm.map -o build/dsky_lamp_test-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_LAMP_TEST_TARGET_CM $(DSKY_LAMP_TEST_FLAGS) $(DSKY_LAMP_TEST_CORE) tests/dsky_lamp_test_check.c tools/dsky_lamp_test_diagnostic.c -Wl,-Map,build/dsky_lamp_test-cm.map -o build/dsky_lamp_test-cm.prg
differential-dsky-lamp-test:
	python3 tools/dsky_lamp_test_differential.py
	python3 tools/freeze_dsky_lamp_test.py
test: test-dsky-lamp-test
sanitize: sanitize-dsky-lamp-test
.PHONY: all test sanitize test-dsky-lamp-test sanitize-dsky-lamp-test c64-dsky-lamp-test differential-dsky-lamp-test
