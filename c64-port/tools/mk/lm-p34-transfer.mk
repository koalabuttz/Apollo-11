# SPDX-License-Identifier: MIT
# Milestone lm-p34-transfer (lane E): Luminary P34-35 GET45 +1 through VNPOOH as control-transfer routines
LM_P34_SRC = src/lm_p34_routines.c src/lm_interbank_routines.c src/lm_interbank.c src/agc_transfer.c src/lm_vn1645.c src/lm_midgim.c src/inverse_trig.c src/guidance_norm.c src/guidance_product.c src/matrix_native.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/rtb_agreement.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_P34_CASES_SRC = tests/lm_p34_cases.c tests/lm_vn1645_cases.c tests/lm_vn1645_debris.c tests/lm_midgim_cases.c tests/lm_midgim_debris.c
LM_P34_C64 = tests/lm_p34_cases.c tests/lm_vn1645_debris.c tests/lm_midgim_debris.c
LM_P34_HDR = include/lm_p34_routines.h include/agc_transfer.h include/lm_interbank_routines.h include/lm_vn1645.h include/lm_midgim.h tests/lm_p34_cases.h tests/lm_p34_vectors.h tests/lm_p34_c64_vectors.h tests/lm_p34_c64_inputs.h
.PHONY: test-lm-p34-transfer sanitize-lm-p34-transfer c64-lm-p34-transfer vice-lm-p34-transfer
all: build/test-lm-p34-transfer
test: test-lm-p34-transfer
sanitize: sanitize-lm-p34-transfer
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/lm-p34-transfer-reference.snapshots.bin.xz: tools/lm_p34_differential.py tools/lm_p34_reference.c tools/lm_p34_snapshot.c tools/lm_p34_inputs.c $(LM_P34_SRC) $(LM_P34_CASES_SRC) | build
	python3 tools/lm_p34_differential.py
build/test-lm-p34-transfer: $(LM_P34_SRC) $(LM_P34_CASES_SRC) tests/test_lm_p34.c $(LM_P34_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_P34_SRC) $(LM_P34_CASES_SRC) tests/test_lm_p34.c -lm -o $@
test-lm-p34-transfer: build/test-lm-p34-transfer build/lm-p34-transfer-reference.snapshots.bin.xz
	./build/test-lm-p34-transfer
	python3 tools/lm_p34_source.py
sanitize-lm-p34-transfer: build/lm-p34-transfer-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_P34_SRC) $(LM_P34_CASES_SRC) tests/test_lm_p34.c -lm -o build/test-lm-p34-transfer-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p34-transfer-sanitize
# Three PRGs, one per case group, each linking only the routines its cases
# reach, with the image under KERNAL ROM.
c64-lm-p34-transfer: build/lm-p34-transfer-1.prg build/lm-p34-transfer-2.prg build/lm-p34-transfer-3.prg
build/lm-p34-transfer-%.prg: $(LM_P34_SRC) $(LM_P34_C64) platform/c64/lm_p34_diagnostic.c $(LM_P34_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_P34_C64_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-p34-transfer-$*.map $(LM_P34_SRC) $(LM_P34_C64) platform/c64/lm_p34_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-p34-transfer-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-p34-transfer: c64-lm-p34-transfer
	python3 tools/vice_run.py build/lm-p34-transfer-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-p34-transfer-2.prg
	python3 tools/vice_run.py build/lm-p34-transfer-3.prg
