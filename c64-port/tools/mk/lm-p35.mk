# SPDX-License-Identifier: MIT
# Milestone lm-p35 (lane E): Luminary P35/P75 as transfer routines
LM_P35_SRC = src/lm_p35_routines.c src/agc_transfer.c src/lm_interp_orders.c src/lm_interp_steps.c src/matrix_native.c src/guidance_product.c src/guidance_norm.c src/inverse_trig.c src/rtb_agreement.c src/agc_image_ops.c src/agc_hardware_divide.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_P35_HDR = include/lm_p35_routines.h include/agc_transfer.h include/lm_interp_steps.h include/lm_interp_orders.h tests/lm_p35_cases.h tests/lm_p35_vectors.h
.PHONY: test-lm-p35 sanitize-lm-p35 c64-lm-p35 vice-lm-p35
all: build/test-lm-p35
test: test-lm-p35
sanitize: sanitize-lm-p35
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/lm-p35-reference.snapshots.bin.xz: tools/lm_p35_differential.py tools/lm_p35_reference.c tools/lm_p35_snapshot.c $(LM_P35_SRC) tests/lm_p35_cases.c | build
	python3 tools/lm_p35_differential.py
build/test-lm-p35: $(LM_P35_SRC) tests/lm_p35_cases.c tests/test_lm_p35.c $(LM_P35_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_P35_SRC) tests/lm_p35_cases.c tests/test_lm_p35.c -lm -o $@
test-lm-p35: build/test-lm-p35 build/lm-p35-reference.snapshots.bin.xz
	./build/test-lm-p35
	python3 tools/lm_p35_source.py
sanitize-lm-p35: build/lm-p35-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_P35_SRC) tests/lm_p35_cases.c tests/test_lm_p35.c -lm -o build/test-lm-p35-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p35-sanitize
# One PRG registering every routine, with the image under KERNAL ROM.
c64-lm-p35: build/lm-p35-1.prg
build/lm-p35-%.prg: $(LM_P35_SRC) tests/lm_p35_cases.c platform/c64/lm_p35_diagnostic.c $(LM_P35_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_P35_C64_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-p35-$*.map $(LM_P35_SRC) tests/lm_p35_cases.c platform/c64/lm_p35_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-p35-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-p35: c64-lm-p35
	python3 tools/vice_run.py build/lm-p35-1.prg --cycles 3000000000 --timeout 1200
