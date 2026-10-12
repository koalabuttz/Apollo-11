# SPDX-License-Identifier: MIT
# Milestone lm-p31 (lane E): Luminary P31 (LAMBERT_AIMPOINT_GUIDANCE) as transfer routines
LM_P31_SRC = src/lm_p31_routines.c src/agc_transfer.c src/lm_interp_orders.c src/lm_interp_steps.c src/matrix_native.c src/guidance_product.c src/guidance_norm.c src/inverse_trig.c src/rtb_agreement.c src/agc_image_ops.c src/agc_hardware_divide.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_P31_HDR = include/lm_p31_routines.h include/agc_transfer.h include/lm_interp_steps.h include/lm_interp_orders.h tests/lm_p31_cases.h tests/lm_p31_vectors.h
.PHONY: test-lm-p31 sanitize-lm-p31 c64-lm-p31 vice-lm-p31
all: build/test-lm-p31
test: test-lm-p31
sanitize: sanitize-lm-p31
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/lm-p31-reference.snapshots.bin.xz: tools/lm_p31_differential.py tools/lm_p31_reference.c tools/lm_p31_snapshot.c $(LM_P31_SRC) tests/lm_p31_cases.c | build
	python3 tools/lm_p31_differential.py
build/test-lm-p31: $(LM_P31_SRC) tests/lm_p31_cases.c tests/test_lm_p31.c $(LM_P31_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_P31_SRC) tests/lm_p31_cases.c tests/test_lm_p31.c -lm -o $@
test-lm-p31: build/test-lm-p31 build/lm-p31-reference.snapshots.bin.xz
	./build/test-lm-p31
	python3 tools/lm_p31_source.py
sanitize-lm-p31: build/lm-p31-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_P31_SRC) tests/lm_p31_cases.c tests/test_lm_p31.c -lm -o build/test-lm-p31-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p31-sanitize
# One PRG registering every routine, with the image under KERNAL ROM.
c64-lm-p31: build/lm-p31-1.prg
build/lm-p31-%.prg: $(LM_P31_SRC) tests/lm_p31_cases.c platform/c64/lm_p31_diagnostic.c $(LM_P31_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_P31_C64_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-p31-$*.map $(LM_P31_SRC) tests/lm_p31_cases.c platform/c64/lm_p31_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-p31-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-p31: c64-lm-p31
	python3 tools/vice_run.py build/lm-p31-1.prg --cycles 3000000000 --timeout 1200
