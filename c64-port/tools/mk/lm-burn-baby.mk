# SPDX-License-Identifier: MIT
# Milestone lm-burn-baby (lane F): Luminary099 BURN_BABY_BURN as native
# routines of the control-transfer ABI.
LM_BB_SRC = src/lm_burn_baby.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_burn_baby_cases.c
LM_BB_HDR = include/lm_burn_baby.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h include/agc_transfer.h tests/lm_burn_baby_cases.h
LM_BB_FLAGS = -Isrc -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-burn-baby sanitize-lm-burn-baby c64-lm-burn-baby vice-lm-burn-baby lm-burn-baby-differential
all: build/test-lm-burn-baby
test: test-lm-burn-baby
sanitize: sanitize-lm-burn-baby
build/test-lm-burn-baby: $(LM_BB_SRC) tests/test_lm_burn_baby.c tests/lm_burn_baby_vectors.h $(LM_BB_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_BB_FLAGS) -DLMBB_COUNT_PATHS $(LM_BB_SRC) tests/test_lm_burn_baby.c -o $@
test-lm-burn-baby: build/test-lm-burn-baby
	./build/test-lm-burn-baby
	python3 tools/lm_burn_baby_source.py
sanitize-lm-burn-baby: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_BB_FLAGS) -DLMBB_COUNT_PATHS -fsanitize=address,undefined $(LM_BB_SRC) tests/test_lm_burn_baby.c -o build/test-lm-burn-baby-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-burn-baby-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-burn-baby-native: $(LM_BB_SRC) platform/host/lm_burn_baby_snapshot.c $(LM_BB_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_BB_FLAGS) $(LM_BB_SRC) platform/host/lm_burn_baby_snapshot.c -o $@
lm-burn-baby-differential: build/lm-burn-baby-native
	python3 tools/lm_burn_baby_freeze.py
	git diff --exit-code -- tests/lm_burn_baby_vectors.h tests/lm_burn_baby_c64_vectors.h
# Two PRGs (LMBB_C64_GROUP): group 2 registers the bank-17 and bank-35
# routines with their code, group 1 bank 36, KILLTSK2 and KILLTASK. Both keep their images
# in the RAM under KERNAL ($e000, undo $f000) and are built without
# inlining; the link fails if code or BSS reaches the mailbox page.
c64-lm-burn-baby: build/lm-burn-baby-1.prg build/lm-burn-baby-2.prg
build/lm-burn-baby-%.prg: $(LM_BB_SRC) platform/c64/lm_burn_baby_diagnostic.c tests/lm_burn_baby_c64_vectors.h $(LM_BB_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Oz -DAPOLLO_FIXED_LM_ONLY -DP66_NATIVE_DISPATCH_ONLY -DLMBB_C64_GROUP=$* -DLMBB_UNDO_ADDRESS=0xf000u -fno-inline -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-burn-baby-$*.map $(LM_BB_SRC) platform/c64/lm_burn_baby_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-burn-baby-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'code and BSS reach the mailbox page: '+hex(e))"
vice-lm-burn-baby: c64-lm-burn-baby
	python3 tools/vice_run.py build/lm-burn-baby-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-burn-baby-2.prg --cycles 3000000000 --timeout 1200
