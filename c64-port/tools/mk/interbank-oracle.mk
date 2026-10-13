# SPDX-License-Identifier: MIT
# Milestone interbank-oracle (lane A): INTER-BANK_COMMUNICATION of both
# missions through the control-transfer ABI, checked against yaAGC.
IBO_SRC = tests/interbank_oracle_cases.c tests/interbank_oracle_check.c src/agc_transfer.c src/lm_interbank_routines.c \
	src/lm_interbank.c src/cm_interbank_routines.c src/cm_interbank_complete.c src/agc_word.c
IBO_HDR = include/agc_transfer.h include/lm_interbank_routines.h include/lm_interbank.h include/cm_interbank_routines.h \
	include/cm_interbank_complete.h src/lm_interbank_sites.inc src/cm_interbank_sites.inc tests/interbank_oracle_cases.h \
	tests/interbank_oracle_check.h tests/interbank_oracle_data.h tests/interbank_oracle_vectors.h
IBO_C64_HDR = tests/interbank_oracle_c64_vectors.h tests/interbank_oracle_c64_rows.h
.PHONY: test-interbank-oracle sanitize-interbank-oracle c64-interbank-oracle vice-interbank-oracle interbank-oracle-differential
all: build/test-interbank-oracle
test: test-interbank-oracle
sanitize: sanitize-interbank-oracle
build/test-interbank-oracle: $(IBO_SRC) tests/test_interbank_oracle.c $(IBO_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(IBO_SRC) tests/test_interbank_oracle.c -o $@
test-interbank-oracle: build/test-interbank-oracle
	./build/test-interbank-oracle
sanitize-interbank-oracle: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(IBO_SRC) tests/test_interbank_oracle.c -o build/test-interbank-oracle-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-interbank-oracle-sanitize
# Host-only oracle run (needs deps/virtualagc and both rope builds).
interbank-oracle-differential:
	python3 tools/interbank_oracle_freeze.py
	git diff --exit-code -- tests/interbank_oracle_vectors.h tests/interbank_oracle_c64_vectors.h tests/interbank_oracle_data.h
c64-interbank-oracle: build/interbank-oracle.prg
build/interbank-oracle.prg: $(IBO_SRC) platform/c64/interbank_oracle_diagnostic.c $(IBO_HDR) $(IBO_C64_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/interbank-oracle.map $(IBO_SRC) platform/c64/interbank_oracle_diagnostic.c -o $@
vice-interbank-oracle: build/interbank-oracle.prg
	python3 tools/vice_run.py build/interbank-oracle.prg --cycles 3000000000 --timeout 1800
