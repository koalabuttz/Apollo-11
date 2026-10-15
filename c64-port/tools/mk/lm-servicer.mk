# SPDX-License-Identifier: MIT
# Milestone lm-servicer (lane F): Luminary099 SERVICER as native
# routines of the control-transfer ABI.
LM_SV_SRC = src/lm_servicer.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_servicer_cases.c
LM_SV_HDR = include/lm_servicer.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h include/agc_transfer.h tests/lm_servicer_cases.h
LM_SV_FLAGS = -Isrc -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-servicer sanitize-lm-servicer c64-lm-servicer vice-lm-servicer lm-servicer-differential
all: build/test-lm-servicer
test: test-lm-servicer
sanitize: sanitize-lm-servicer
build/test-lm-servicer: $(LM_SV_SRC) tests/test_lm_servicer.c tests/lm_servicer_vectors.h $(LM_SV_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_SV_FLAGS) -DLMSV_COUNT_PATHS $(LM_SV_SRC) tests/test_lm_servicer.c -o $@
test-lm-servicer: build/test-lm-servicer
	./build/test-lm-servicer
	python3 tools/lm_servicer_source.py
	python3 tools/flight_exit_returns.py
sanitize-lm-servicer: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_SV_FLAGS) -DLMSV_COUNT_PATHS -fsanitize=address,undefined $(LM_SV_SRC) tests/test_lm_servicer.c -o build/test-lm-servicer-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-servicer-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-servicer-native: $(LM_SV_SRC) platform/host/lm_servicer_snapshot.c $(LM_SV_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_SV_FLAGS) $(LM_SV_SRC) platform/host/lm_servicer_snapshot.c -o $@
lm-servicer-differential: build/lm-servicer-native
	python3 tools/lm_servicer_freeze.py
	git diff --exit-code -- tests/lm_servicer_vectors.h tests/lm_servicer_c64_vectors.h
# Seven PRGs, one per C64 segment group (LMSV_C64_GROUP, src/lm_servicer.c):
# group 1 holds the basic code outside the interpretive passages, groups
# 2-7 the passages (with the basic code between them); each runs its own
# keys' rows to the end of the group's code. All keep their images in the
# RAM under KERNAL ($e000, undo $f000) and are built without inlining and
# with llvm-mos's static stack (it saves about 1.7 KB); the link fails if
# code or BSS reaches the mailbox page.
LM_SV_GROUPS = 1 2 3 4 5 6 7
c64-lm-servicer: $(foreach g,$(LM_SV_GROUPS),build/lm-servicer-$(g).prg)
build/lm-servicer-%.prg: $(LM_SV_SRC) platform/c64/lm_servicer_diagnostic.c tests/lm_servicer_c64_vectors.h $(LM_SV_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Oz -DAPOLLO_FIXED_LM_ONLY -DP66_NATIVE_DISPATCH_ONLY -DLMSV_C64_GROUP=$* -DLMSV_UNDO_ADDRESS=0xf000u -Isrc -fno-inline -ffunction-sections -fdata-sections -Wl,--gc-sections -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-servicer-$*.map $(LM_SV_SRC) platform/c64/lm_servicer_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-servicer-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'code and BSS reach the mailbox page: '+hex(e))"
vice-lm-servicer: c64-lm-servicer
	$(foreach g,$(LM_SV_GROUPS),python3 tools/vice_run.py build/lm-servicer-$(g).prg --cycles 3000000000 --timeout 1200 &&) true
