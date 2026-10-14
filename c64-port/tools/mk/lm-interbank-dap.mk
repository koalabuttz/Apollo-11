# SPDX-License-Identifier: MIT
# The DAP excursion's IBNKCALL/ISWRETRN table (lane D2, docs/FLIGHT-STORE.md):
# src/lm_interbank_dap.c against lm_interbank_routines on random states.
LM_IBD_SRC = src/lm_interbank_dap.c src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c src/agc_transfer.c src/agc_word.c
LM_IBD_HDR = include/lm_interbank_dap.h include/lm_interbank_routines.h include/lm_interbank.h include/agc_transfer.h
.PHONY: test-lm-interbank-dap sanitize-lm-interbank-dap c64-lm-interbank-dap
all: build/test-lm-interbank-dap
test: test-lm-interbank-dap
sanitize: sanitize-lm-interbank-dap
build/test-lm-interbank-dap: $(LM_IBD_SRC) tests/test_lm_interbank_dap.c $(LM_IBD_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_IBD_SRC) tests/test_lm_interbank_dap.c -o $@
test-lm-interbank-dap: build/test-lm-interbank-dap
	./build/test-lm-interbank-dap
sanitize-lm-interbank-dap: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_IBD_SRC) tests/test_lm_interbank_dap.c -o build/test-lm-interbank-dap-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-interbank-dap-sanitize
