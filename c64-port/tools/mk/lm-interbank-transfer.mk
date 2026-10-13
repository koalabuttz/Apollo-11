# SPDX-License-Identifier: MIT
# Milestone lm-interbank-transfer (lane A): Luminary099 INTER-BANK_COMMUNICATION
# entries as native routines of the control-transfer ABI.
LM_IBR_SRC = src/agc_transfer.c src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c src/agc_word.c tests/lm_interbank_routines_cases.c
LM_IBR_HDR = include/agc_transfer.h include/lm_interbank_routines.h include/lm_interbank.h src/lm_interbank_sites.inc tests/lm_interbank_routines_cases.h
.PHONY: test-lm-interbank-transfer sanitize-lm-interbank-transfer c64-lm-interbank-transfer vice-lm-interbank-transfer
all: build/test-lm-interbank-transfer
test: test-lm-interbank-transfer
sanitize: sanitize-lm-interbank-transfer
build/test-lm-interbank-transfer: $(LM_IBR_SRC) src/lm_fixed_compat.c tests/test_lm_interbank_routines.c $(LM_IBR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_IBR_SRC) src/lm_fixed_compat.c tests/test_lm_interbank_routines.c -o $@
test-lm-interbank-transfer: build/test-lm-interbank-transfer
	python3 tools/agc_sites_generate.py lm-interbank --check
	./build/test-lm-interbank-transfer
sanitize-lm-interbank-transfer: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_IBR_SRC) src/lm_fixed_compat.c tests/test_lm_interbank_routines.c -o build/test-lm-interbank-transfer-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-interbank-transfer-sanitize
build/lm-interbank-transfer-digest: $(LM_IBR_SRC) platform/host/lm_interbank_routines_digest.c $(LM_IBR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_IBR_SRC) platform/host/lm_interbank_routines_digest.c -o $@
build/lm-interbank-transfer-vectors.h: build/lm-interbank-transfer-digest
	./build/lm-interbank-transfer-digest > $@
c64-lm-interbank-transfer: build/lm-interbank-transfer.prg
build/lm-interbank-transfer.prg: $(LM_IBR_SRC) platform/c64/lm_interbank_routines_diagnostic.c $(LM_IBR_HDR) build/lm-interbank-transfer-vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -Ibuild -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-interbank-transfer.map $(LM_IBR_SRC) platform/c64/lm_interbank_routines_diagnostic.c -o $@
vice-lm-interbank-transfer: build/lm-interbank-transfer.prg
	python3 tools/vice_run.py build/lm-interbank-transfer.prg --cycles 3000000000 --timeout 1200
