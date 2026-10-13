# SPDX-License-Identifier: MIT
# Milestone standard-lead-oracle (lane A): SYSTEM_TEST_STANDARD_LEAD_INS of
# both missions through the control-transfer ABI, checked against yaAGC.
SLO_SRC = tests/standard_lead_oracle_cases.c tests/standard_lead_oracle_check.c tests/standard_lead_cases.c \
	src/agc_transfer.c src/standard_lead_routines.c src/standard_lead.c src/standard_lead_interp.c \
	src/standard_lead_wake.c src/lm_basic_wake.c src/executive.c src/cm_basic_wake.c src/cm_native_image.c \
	src/cm_dispatch.c src/cm_executive.c src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c src/cm_interbank_routines.c src/cm_interbank_idle.c \
	src/cm_interbank_complete.c src/agc_word.c
SLO_HDR = include/agc_transfer.h include/standard_lead_routines.h include/standard_lead.h include/standard_lead_wake.h \
	include/lm_interbank_routines.h include/cm_interbank_routines.h tests/standard_lead_oracle_cases.h \
	tests/standard_lead_oracle_check.h tests/standard_lead_oracle_vectors.h tests/standard_lead_cases.h \
	tests/standard_lead_fixed_inputs.h
SLO_C64_HDR = tests/standard_lead_oracle_c64_vectors.h tests/standard_lead_oracle_c64_rows.h
.PHONY: test-standard-lead-oracle sanitize-standard-lead-oracle c64-standard-lead-oracle vice-standard-lead-oracle standard-lead-oracle-differential
all: build/test-standard-lead-oracle
test: test-standard-lead-oracle
sanitize: sanitize-standard-lead-oracle
build/test-standard-lead-oracle: $(SLO_SRC) tests/test_standard_lead_oracle.c $(SLO_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SLO_SRC) tests/test_standard_lead_oracle.c -o $@
test-standard-lead-oracle: build/test-standard-lead-oracle
	./build/test-standard-lead-oracle
sanitize-standard-lead-oracle: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(SLO_SRC) tests/test_standard_lead_oracle.c -o build/test-standard-lead-oracle-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-standard-lead-oracle-sanitize
# Host-only oracle run (needs deps/virtualagc and both rope builds).
standard-lead-oracle-differential:
	python3 tools/standard_lead_oracle_freeze.py
	git diff --exit-code -- tests/standard_lead_oracle_vectors.h tests/standard_lead_oracle_c64_vectors.h
c64-standard-lead-oracle: build/standard-lead-oracle.prg
build/standard-lead-oracle.prg: $(SLO_SRC) platform/c64/standard_lead_oracle_diagnostic.c $(SLO_HDR) $(SLO_C64_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/standard-lead-oracle.map $(SLO_SRC) platform/c64/standard_lead_oracle_diagnostic.c -o $@
vice-standard-lead-oracle: build/standard-lead-oracle.prg
	python3 tools/vice_run.py build/standard-lead-oracle.prg --cycles 3000000000 --timeout 1800
