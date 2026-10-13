# SPDX-License-Identifier: MIT
# Milestone lm-t4rupt-chain (lane A): whole T4RUPT interrupts of Luminary099
# through the trampoline over the three routine tables of T4RUPT_PROGRAM.agc.
LCH_SRC = src/t4rupt_core.c src/lm_t4rupt_imu.c src/lm_t4rupt_rr.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_t4rupt_chain_cases.c
LCH_HDR = include/t4rupt_core.h include/lm_t4rupt_imu.h include/lm_t4rupt_rr.h include/agc_transfer.h include/agc_image_ops.h tests/lm_t4rupt_chain_cases.h
LCH_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-t4rupt-chain sanitize-lm-t4rupt-chain c64-lm-t4rupt-chain vice-lm-t4rupt-chain lm-t4rupt-chain-differential
all: build/test-lm-t4rupt-chain
test: test-lm-t4rupt-chain
sanitize: sanitize-lm-t4rupt-chain
build/test-lm-t4rupt-chain: $(LCH_SRC) tests/test_lm_t4rupt_chain.c tests/lm_t4rupt_chain_vectors.h $(LCH_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LCH_FLAGS) $(LCH_SRC) tests/test_lm_t4rupt_chain.c -o $@
test-lm-t4rupt-chain: build/test-lm-t4rupt-chain
	./build/test-lm-t4rupt-chain
sanitize-lm-t4rupt-chain: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LCH_FLAGS) -fsanitize=address,undefined $(LCH_SRC) tests/test_lm_t4rupt_chain.c -o build/test-lm-t4rupt-chain-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-t4rupt-chain-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-t4rupt-chain-native: $(LCH_SRC) platform/host/lm_t4rupt_chain_snapshot.c $(LCH_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LCH_FLAGS) $(LCH_SRC) platform/host/lm_t4rupt_chain_snapshot.c -o $@
lm-t4rupt-chain-differential: build/lm-t4rupt-chain-native
	python3 tools/lm_t4rupt_chain_freeze.py
	git diff --exit-code -- tests/lm_t4rupt_chain_vectors.h tests/lm_t4rupt_chain_c64_vectors.h
c64-lm-t4rupt-chain: build/lm-t4rupt-chain.prg
build/lm-t4rupt-chain.prg: $(LCH_SRC) platform/c64/lm_t4rupt_chain_diagnostic.c tests/lm_t4rupt_chain_c64_vectors.h tests/lm_t4rupt_chain_c64_rows.h $(LCH_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(LCH_FLAGS) -Wl,-Map,build/lm-t4rupt-chain.map $(LCH_SRC) platform/c64/lm_t4rupt_chain_diagnostic.c -o $@
vice-lm-t4rupt-chain: build/lm-t4rupt-chain.prg
	python3 tools/vice_run.py build/lm-t4rupt-chain.prg --cycles 3000000000 --timeout 1200
