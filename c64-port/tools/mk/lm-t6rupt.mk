# SPDX-License-Identifier: MIT
# Milestone lm-t6rupt (lane A): Luminary099 T6-RUPT_PROGRAMS (the T6RUPT
# vector, DOT6RUPT, T6JOBCHK/T6JOB, WRITEP/WRITEU/WRITEV) and the WAITLIST
# RESUME/NOQRSM/NOQBRSM exits as native routines (lm_t6rupt_routines).
LM_T6_SRC = src/lm_t6rupt.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_t6rupt_cases.c
LM_T6_HDR = tests/resume_rope_words.h include/lm_t6rupt_routines.h include/agc_transfer.h include/agc_image_ops.h tests/lm_t6rupt_cases.h
LM_T6_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-t6rupt sanitize-lm-t6rupt c64-lm-t6rupt vice-lm-t6rupt lm-t6rupt-differential
all: build/test-lm-t6rupt
test: test-lm-t6rupt
sanitize: sanitize-lm-t6rupt
build/test-lm-t6rupt: $(LM_T6_SRC) tests/test_lm_t6rupt.c tests/lm_t6rupt_vectors.h $(LM_T6_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_T6_FLAGS) $(LM_T6_SRC) tests/test_lm_t6rupt.c -o $@
test-lm-t6rupt: build/test-lm-t6rupt
	./build/test-lm-t6rupt
sanitize-lm-t6rupt: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_T6_FLAGS) -fsanitize=address,undefined $(LM_T6_SRC) tests/test_lm_t6rupt.c -o build/test-lm-t6rupt-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-t6rupt-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-t6rupt-native: $(LM_T6_SRC) platform/host/lm_t6rupt_snapshot.c $(LM_T6_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_T6_FLAGS) $(LM_T6_SRC) platform/host/lm_t6rupt_snapshot.c -o $@
lm-t6rupt-differential: build/lm-t6rupt-native
	python3 tools/lm_t6rupt_freeze.py
	python3 tools/lm_t6rupt_source_map.py
	git diff --exit-code -- tests/lm_t6rupt_vectors.h tests/lm_t6rupt_c64_vectors.h
c64-lm-t6rupt: build/lm-t6rupt.prg
build/lm-t6rupt.prg: $(LM_T6_SRC) platform/c64/lm_t6rupt_diagnostic.c tests/lm_t6rupt_c64_vectors.h tests/lm_t6rupt_c64_rows.h $(LM_T6_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(LM_T6_FLAGS) -Wl,-Map,build/lm-t6rupt.map $(LM_T6_SRC) platform/c64/lm_t6rupt_diagnostic.c -o $@
vice-lm-t6rupt: build/lm-t6rupt.prg
	python3 tools/vice_run.py build/lm-t6rupt.prg --cycles 3000000000 --timeout 1200
