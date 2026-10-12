# SPDX-License-Identifier: MIT
# Milestone lm-standard-lead-transfer (lane A): Luminary099
# SYSTEM_TEST_STANDARD_LEAD_INS entries as native routines of the
# control-transfer ABI, with E/BKCALL and E/CALL round trips.
LM_SLR_SRC = src/agc_transfer.c src/standard_lead_routines.c src/standard_lead.c src/standard_lead_interp.c src/standard_lead_wake.c src/lm_basic_wake.c src/executive.c src/cm_basic_wake.c src/cm_native_image.c src/cm_dispatch.c src/cm_executive.c src/lm_interbank_routines.c src/lm_interbank.c src/agc_word.c tests/standard_lead_routines_cases.c tests/standard_lead_cases.c
LM_SLR_HDR = include/agc_transfer.h include/standard_lead_routines.h include/standard_lead.h include/lm_interbank_routines.h src/lm_interbank_sites.inc tests/standard_lead_routines_cases.h tests/standard_lead_fixed_inputs.h
.PHONY: test-lm-standard-lead-transfer sanitize-lm-standard-lead-transfer c64-lm-standard-lead-transfer vice-lm-standard-lead-transfer
all: build/test-lm-standard-lead-transfer
test: test-lm-standard-lead-transfer
sanitize: sanitize-lm-standard-lead-transfer
build/test-lm-standard-lead-transfer: $(LM_SLR_SRC) tests/test_standard_lead_routines.c $(LM_SLR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_SLR_SRC) tests/test_standard_lead_routines.c -o $@
test-lm-standard-lead-transfer: build/test-lm-standard-lead-transfer
	./build/test-lm-standard-lead-transfer
sanitize-lm-standard-lead-transfer: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_SLR_SRC) tests/test_standard_lead_routines.c -o build/test-lm-standard-lead-transfer-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-standard-lead-transfer-sanitize
build/lm-standard-lead-transfer-vectors.h: build/test-lm-standard-lead-transfer
	./build/test-lm-standard-lead-transfer --vectors > $@
c64-lm-standard-lead-transfer: build/lm-standard-lead-transfer.prg
build/lm-standard-lead-transfer.prg: $(LM_SLR_SRC) platform/c64/standard_lead_routines_diagnostic.c $(LM_SLR_HDR) build/lm-standard-lead-transfer-vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -Ibuild -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-standard-lead-transfer.map $(LM_SLR_SRC) platform/c64/standard_lead_routines_diagnostic.c -o $@
vice-lm-standard-lead-transfer: build/lm-standard-lead-transfer.prg
	python3 tools/vice_run.py build/lm-standard-lead-transfer.prg --cycles 3000000000 --timeout 1200
