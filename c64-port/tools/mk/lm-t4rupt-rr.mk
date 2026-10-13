# SPDX-License-Identifier: MIT
# Milestone lm-t4rupt-rr (lane A): the rendezvous radar monitor and GPMATRIX
# of Luminary099 T4RUPT_PROGRAM.agc (06,3006-3155) as lm_t4rupt_rr_routines.
LRR_SRC = src/lm_t4rupt_rr.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_t4rupt_rr_cases.c
LRR_HDR = include/lm_t4rupt_rr.h include/agc_transfer.h include/agc_image_ops.h tests/lm_t4rupt_rr_cases.h
LRR_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-t4rupt-rr sanitize-lm-t4rupt-rr c64-lm-t4rupt-rr vice-lm-t4rupt-rr lm-t4rupt-rr-differential
all: build/test-lm-t4rupt-rr
test: test-lm-t4rupt-rr
sanitize: sanitize-lm-t4rupt-rr
build/test-lm-t4rupt-rr: $(LRR_SRC) tests/test_lm_t4rupt_rr.c tests/lm_t4rupt_rr_vectors.h $(LRR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LRR_FLAGS) $(LRR_SRC) tests/test_lm_t4rupt_rr.c -o $@
test-lm-t4rupt-rr: build/test-lm-t4rupt-rr
	./build/test-lm-t4rupt-rr
sanitize-lm-t4rupt-rr: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LRR_FLAGS) -fsanitize=address,undefined $(LRR_SRC) tests/test_lm_t4rupt_rr.c -o build/test-lm-t4rupt-rr-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-t4rupt-rr-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-t4rupt-rr-native: $(LRR_SRC) platform/host/lm_t4rupt_rr_snapshot.c $(LRR_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LRR_FLAGS) $(LRR_SRC) platform/host/lm_t4rupt_rr_snapshot.c -o $@
lm-t4rupt-rr-differential: build/lm-t4rupt-rr-native
	python3 tools/lm_t4rupt_rr_freeze.py
	python3 tools/lm_t4rupt_rr_source_map.py
	git diff --exit-code -- tests/lm_t4rupt_rr_vectors.h tests/lm_t4rupt_rr_c64_vectors.h
c64-lm-t4rupt-rr: build/lm-t4rupt-rr.prg
build/lm-t4rupt-rr.prg: $(LRR_SRC) platform/c64/lm_t4rupt_rr_diagnostic.c tests/lm_t4rupt_rr_c64_vectors.h $(LRR_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(LRR_FLAGS) -Wl,-Map,build/lm-t4rupt-rr.map $(LRR_SRC) platform/c64/lm_t4rupt_rr_diagnostic.c -o $@
vice-lm-t4rupt-rr: build/lm-t4rupt-rr.prg
	python3 tools/vice_run.py build/lm-t4rupt-rr.prg --cycles 3000000000 --timeout 1200
