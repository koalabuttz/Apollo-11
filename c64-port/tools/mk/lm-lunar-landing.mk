# SPDX-License-Identifier: MIT
# Milestone lm-lunar-landing (lane F): Luminary099 THE_LUNAR_LANDING as native
# routines of the control-transfer ABI.
LM_LLD_SRC = src/lm_lunar_landing.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_lunar_landing_cases.c
LM_LLD_HDR = include/lm_lunar_landing.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h include/agc_transfer.h tests/lm_lunar_landing_cases.h
LM_LLD_FLAGS = -Isrc -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-lunar-landing sanitize-lm-lunar-landing c64-lm-lunar-landing vice-lm-lunar-landing lm-lunar-landing-differential
all: build/test-lm-lunar-landing
test: test-lm-lunar-landing
sanitize: sanitize-lm-lunar-landing
build/test-lm-lunar-landing: $(LM_LLD_SRC) tests/test_lm_lunar_landing.c tests/lm_lunar_landing_vectors.h $(LM_LLD_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_LLD_FLAGS) -DLLD_COUNT_PATHS $(LM_LLD_SRC) tests/test_lm_lunar_landing.c -o $@
test-lm-lunar-landing: build/test-lm-lunar-landing
	./build/test-lm-lunar-landing
	python3 tools/lm_lunar_landing_source.py
sanitize-lm-lunar-landing: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_LLD_FLAGS) -DLLD_COUNT_PATHS -fsanitize=address,undefined $(LM_LLD_SRC) tests/test_lm_lunar_landing.c -o build/test-lm-lunar-landing-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-lunar-landing-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-lunar-landing-native: $(LM_LLD_SRC) platform/host/lm_lunar_landing_snapshot.c $(LM_LLD_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_LLD_FLAGS) $(LM_LLD_SRC) platform/host/lm_lunar_landing_snapshot.c -o $@
lm-lunar-landing-differential: build/lm-lunar-landing-native
	python3 tools/lm_lunar_landing_freeze.py
	git diff --exit-code -- tests/lm_lunar_landing_vectors.h tests/lm_lunar_landing_c64_vectors.h
# Two PRGs (LLD_C64_GROUP): group 2 registers DDUMCALC and P63SPOT2 with
# the code they reach, group 1 every other routine. Both keep their images
# in the RAM under KERNAL ($e000, undo $f000) and are built without
# inlining; the link fails if code or BSS reaches the mailbox page.
c64-lm-lunar-landing: build/lm-lunar-landing-1.prg build/lm-lunar-landing-2.prg
build/lm-lunar-landing-%.prg: $(LM_LLD_SRC) platform/c64/lm_lunar_landing_diagnostic.c tests/lm_lunar_landing_c64_vectors.h $(LM_LLD_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Oz -DAPOLLO_FIXED_LM_ONLY -DP66_NATIVE_DISPATCH_ONLY -DLLD_C64_GROUP=$* -DLLD_UNDO_ADDRESS=0xf000u -fno-inline -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-lunar-landing-$*.map $(LM_LLD_SRC) platform/c64/lm_lunar_landing_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-lunar-landing-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'code and BSS reach the mailbox page: '+hex(e))"
vice-lm-lunar-landing: c64-lm-lunar-landing
	python3 tools/vice_run.py build/lm-lunar-landing-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-lunar-landing-2.prg --cycles 3000000000 --timeout 1200
