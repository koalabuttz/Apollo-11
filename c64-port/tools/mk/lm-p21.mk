# SPDX-License-Identifier: MIT
# Milestone lm-p21 (lane E): Luminary P21 (GROUND_TRACKING_DETERMINATION_PROGRAM) as transfer routines
LM_P21_SRC = src/lm_p21_routines.c src/lm_work_matrix.c src/agc_transfer.c src/lm_interp_orders.c src/lm_interp_steps.c src/matrix_native.c src/guidance_product.c src/guidance_norm.c src/inverse_trig.c src/rtb_agreement.c src/agc_image_ops.c src/agc_hardware_divide.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_P21_HDR = include/lm_p21_routines.h include/agc_transfer.h include/lm_interp_steps.h include/lm_interp_orders.h include/lm_work_matrix.h tests/lm_p21_cases.h tests/lm_p21_vectors.h
.PHONY: test-lm-p21 sanitize-lm-p21 c64-lm-p21 vice-lm-p21
all: build/test-lm-p21
test: test-lm-p21
sanitize: sanitize-lm-p21
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/lm-p21-reference.snapshots.bin.xz: tools/lm_p21_differential.py tools/lm_p21_reference.c tools/lm_p21_snapshot.c $(LM_P21_SRC) tests/lm_p21_cases.c | build
	python3 tools/lm_p21_differential.py
build/test-lm-p21: $(LM_P21_SRC) tests/lm_p21_cases.c tests/test_lm_p21.c $(LM_P21_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_P21_SRC) tests/lm_p21_cases.c tests/test_lm_p21.c -lm -o $@
test-lm-p21: build/test-lm-p21 build/lm-p21-reference.snapshots.bin.xz
	./build/test-lm-p21
	python3 tools/lm_p21_source.py
sanitize-lm-p21: build/lm-p21-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_P21_SRC) tests/lm_p21_cases.c tests/test_lm_p21.c -lm -o build/test-lm-p21-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p21-sanitize
# Two PRGs, each registering only the routines its cases reach, with the
# image under KERNAL ROM.
c64-lm-p21: build/lm-p21-1.prg build/lm-p21-2.prg
build/lm-p21-%.prg: $(LM_P21_SRC) tests/lm_p21_cases.c platform/c64/lm_p21_diagnostic.c $(LM_P21_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DLM_P21_C64_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-p21-$*.map $(LM_P21_SRC) tests/lm_p21_cases.c platform/c64/lm_p21_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-p21-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-p21: c64-lm-p21
	python3 tools/vice_run.py build/lm-p21-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-p21-2.prg --cycles 3000000000 --timeout 1200
