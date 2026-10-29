# SPDX-License-Identifier: MIT
STANDARD_LEAD_INTEGRATION_CORE = src/standard_lead.c src/standard_lead_interp.c src/standard_lead_bank.c src/standard_lead_wake.c src/standard_lead_cm.c src/cm_interbank_extended.c src/lm_basic_wake.c src/executive.c src/cm_basic_wake.c src/cm_native_image.c src/cm_dispatch.c src/cm_executive.c src/agc_word.c tests/standard_lead_cases.c tests/standard_lead_bank_cases.c tests/standard_lead_interp_cases.c tests/standard_lead_wake_cases.c tests/standard_lead_cm_cases.c tests/standard_lead_resume_cases.c tests/standard_lead_target_input.c
all: build/test-standard-lead-integrated
build/test-standard-lead-integrated: tests/test_standard_lead.c $(STANDARD_LEAD_INTEGRATION_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
test: test-standard-lead-integrated
test-standard-lead-integrated: build/test-standard-lead-integrated
	./build/test-standard-lead-integrated
sanitize: sanitize-standard-lead-integrated
sanitize-standard-lead-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -ffunction-sections -Wl,--gc-sections -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_standard_lead.c $(STANDARD_LEAD_INTEGRATION_CORE) -o build/test-standard-lead-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-standard-lead-integrated-san
.PHONY: test-standard-lead-integrated sanitize-standard-lead-integrated
