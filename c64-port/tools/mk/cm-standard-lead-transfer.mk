# SPDX-License-Identifier: MIT
# Milestone cm-standard-lead-transfer (lane A): Comanche055
# SYSTEM_TEST_STANDARD_LEAD_INS entries as native routines of the
# control-transfer ABI, with E/BKCALL and E/CALL round trips.
CM_SLR_SRC = src/agc_transfer.c src/standard_lead_routines.c src/interp_entry.c src/standard_lead.c src/standard_lead_interp.c src/standard_lead_wake.c src/lm_basic_wake.c src/executive.c src/cm_basic_wake.c src/cm_native_image.c src/cm_dispatch.c src/cm_executive.c src/cm_interbank_routines.c src/cm_interbank_idle.c src/cm_interbank_complete.c src/agc_word.c tests/cm_standard_lead_routines_cases.c tests/standard_lead_cases.c
CM_SLR_HDR = include/agc_transfer.h include/standard_lead_routines.h include/standard_lead.h include/cm_interbank_routines.h src/cm_interbank_sites.inc tests/cm_standard_lead_routines_cases.h tests/standard_lead_fixed_inputs.h
.PHONY: test-cm-standard-lead-transfer sanitize-cm-standard-lead-transfer c64-cm-standard-lead-transfer vice-cm-standard-lead-transfer
all: build/test-cm-standard-lead-transfer
test: test-cm-standard-lead-transfer
sanitize: sanitize-cm-standard-lead-transfer
build/test-cm-standard-lead-transfer: $(CM_SLR_SRC) tests/test_cm_standard_lead_routines.c $(CM_SLR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_SLR_SRC) tests/test_cm_standard_lead_routines.c -o $@
test-cm-standard-lead-transfer: build/test-cm-standard-lead-transfer
	./build/test-cm-standard-lead-transfer
sanitize-cm-standard-lead-transfer: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_SLR_SRC) tests/test_cm_standard_lead_routines.c -o build/test-cm-standard-lead-transfer-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-standard-lead-transfer-sanitize
build/cm-standard-lead-transfer-vectors.h: build/test-cm-standard-lead-transfer
	./build/test-cm-standard-lead-transfer --vectors > $@
c64-cm-standard-lead-transfer: build/cm-standard-lead-transfer.prg
build/cm-standard-lead-transfer.prg: $(CM_SLR_SRC) platform/c64/cm_standard_lead_routines_diagnostic.c $(CM_SLR_HDR) build/cm-standard-lead-transfer-vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -Ibuild -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-standard-lead-transfer.map $(CM_SLR_SRC) platform/c64/cm_standard_lead_routines_diagnostic.c -o $@
vice-cm-standard-lead-transfer: build/cm-standard-lead-transfer.prg
	python3 tools/vice_run.py build/cm-standard-lead-transfer.prg --cycles 3000000000 --timeout 1200
