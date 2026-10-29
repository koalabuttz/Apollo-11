# SPDX-License-Identifier: MIT
# Milestone cm-interbank-transfer (lane A): Comanche055 INTER-BANK_COMMUNICATION
# entries as native routines of the control-transfer ABI.
CM_IBR_SRC = src/agc_transfer.c src/cm_interbank_routines.c src/cm_interbank_idle.c src/cm_interbank_complete.c src/agc_word.c tests/cm_interbank_routines_cases.c
CM_IBR_HDR = include/agc_transfer.h include/cm_interbank_routines.h include/cm_interbank_complete.h src/cm_interbank_sites.inc tests/cm_interbank_routines_cases.h
.PHONY: test-cm-interbank-transfer sanitize-cm-interbank-transfer c64-cm-interbank-transfer vice-cm-interbank-transfer
all: build/test-cm-interbank-transfer
test: test-cm-interbank-transfer
sanitize: sanitize-cm-interbank-transfer
build/test-cm-interbank-transfer: $(CM_IBR_SRC) src/cm_fixed_compat.c tests/test_cm_interbank_routines.c $(CM_IBR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IBR_SRC) src/cm_fixed_compat.c tests/test_cm_interbank_routines.c -o $@
test-cm-interbank-transfer: build/test-cm-interbank-transfer
	python3 tools/agc_sites_generate.py cm-interbank --check
	./build/test-cm-interbank-transfer
sanitize-cm-interbank-transfer: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_IBR_SRC) src/cm_fixed_compat.c tests/test_cm_interbank_routines.c -o build/test-cm-interbank-transfer-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-interbank-transfer-sanitize
build/cm-interbank-transfer-digest: $(CM_IBR_SRC) platform/host/cm_interbank_routines_digest.c $(CM_IBR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_IBR_SRC) platform/host/cm_interbank_routines_digest.c -o $@
build/cm-interbank-transfer-vectors.h: build/cm-interbank-transfer-digest
	./build/cm-interbank-transfer-digest > $@
c64-cm-interbank-transfer: build/cm-interbank-transfer.prg
build/cm-interbank-transfer.prg: $(CM_IBR_SRC) platform/c64/cm_interbank_routines_diagnostic.c $(CM_IBR_HDR) build/cm-interbank-transfer-vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -Ibuild -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-interbank-transfer.map $(CM_IBR_SRC) platform/c64/cm_interbank_routines_diagnostic.c -o $@
vice-cm-interbank-transfer: build/cm-interbank-transfer.prg
	python3 tools/vice_run.py build/cm-interbank-transfer.prg --cycles 3000000000 --timeout 1200
