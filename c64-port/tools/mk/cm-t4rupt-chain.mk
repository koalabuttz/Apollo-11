# SPDX-License-Identifier: MIT
# Milestone cm-t4rupt-chain (lane A): whole T4RUPT interrupts of Comanche055
# through the trampoline over the three routine tables of T4RUPT_PROGRAM.agc.
CCH_SRC = src/t4rupt_core.c src/cm_t4rupt_imu.c src/cm_t4rupt_optics.c src/cm_interp_orders.c src/cm_interp_steps.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_t4rupt_chain_cases.c
CCH_HDR = include/t4rupt_core.h include/cm_t4rupt_imu.h include/cm_t4rupt_optics.h include/agc_transfer.h include/agc_image_ops.h tests/cm_t4rupt_chain_cases.h
CCH_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
# The C64 build leaves out the Luminary interpreter orders the host links.
CCH_C64_SRC = $(filter-out src/lm_interp_orders.c src/lm_interp_steps.c,$(CCH_SRC))
.PHONY: test-cm-t4rupt-chain sanitize-cm-t4rupt-chain c64-cm-t4rupt-chain vice-cm-t4rupt-chain cm-t4rupt-chain-differential
all: build/test-cm-t4rupt-chain
test: test-cm-t4rupt-chain
sanitize: sanitize-cm-t4rupt-chain
build/test-cm-t4rupt-chain: $(CCH_SRC) tests/test_cm_t4rupt_chain.c tests/cm_t4rupt_chain_vectors.h $(CCH_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CCH_FLAGS) $(CCH_SRC) tests/test_cm_t4rupt_chain.c -o $@
test-cm-t4rupt-chain: build/test-cm-t4rupt-chain
	./build/test-cm-t4rupt-chain
sanitize-cm-t4rupt-chain: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(CCH_FLAGS) -fsanitize=address,undefined $(CCH_SRC) tests/test_cm_t4rupt_chain.c -o build/test-cm-t4rupt-chain-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-t4rupt-chain-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-t4rupt-chain-native: $(CCH_SRC) platform/host/cm_t4rupt_chain_snapshot.c $(CCH_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CCH_FLAGS) $(CCH_SRC) platform/host/cm_t4rupt_chain_snapshot.c -o $@
cm-t4rupt-chain-differential: build/cm-t4rupt-chain-native
	python3 tools/cm_t4rupt_chain_freeze.py
	git diff --exit-code -- tests/cm_t4rupt_chain_vectors.h tests/cm_t4rupt_chain_c64_vectors.h
c64-cm-t4rupt-chain: build/cm-t4rupt-chain.prg
build/cm-t4rupt-chain.prg: $(CCH_SRC) platform/c64/cm_t4rupt_chain_diagnostic.c tests/cm_t4rupt_chain_c64_vectors.h tests/cm_t4rupt_chain_c64_rows.h $(CCH_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Oz -fno-inline -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(CCH_FLAGS) -Wl,-Map,build/cm-t4rupt-chain.map $(CCH_C64_SRC) platform/c64/cm_t4rupt_chain_diagnostic.c -o $@
vice-cm-t4rupt-chain: build/cm-t4rupt-chain.prg
	python3 tools/vice_run.py build/cm-t4rupt-chain.prg --cycles 3000000000 --timeout 1200
# Host-only: the whole-file source map. Reruns the cm-t4rupt-optics
# differential under tools/oracle_trace for the optics routines' executed
# words; t4rupt-core, cm-t4rupt-imu and this milestone write their own.
.PHONY: cm-t4rupt-closure-trace
cm-t4rupt-closure-trace: build/t4rupt-core-native build/cm-t4rupt-imu-native build/cm-t4rupt-chain-native | build
	python3 tools/t4rupt_core_freeze.py
	python3 tools/cm_t4rupt_imu_freeze.py
	python3 tools/cm_t4rupt_chain_freeze.py
	rm -f build/cm-t4rupt-optics-executed.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/cm-t4rupt-optics-executed.txt $(MAKE) --no-print-directory cm-t4rupt-optics-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/cm_t4rupt_closure_source_map.py
