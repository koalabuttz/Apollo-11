# SPDX-License-Identifier: MIT
# Milestone lm-p33 (lane E): Luminary P33/P73, AVFLAGA, AVFLAGP, P20FLGON and DISDVLVC as transfer routines
LM_P33_SRC = src/lm_p33_routines.c src/lm_work_matrix.c src/agc_transfer.c src/lm_interp_orders.c src/lm_interp_steps.c src/matrix_native.c src/guidance_product.c src/guidance_norm.c src/inverse_trig.c src/rtb_agreement.c src/agc_image_ops.c src/agc_hardware_divide.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_P33_HDR = include/lm_p33_routines.h include/agc_transfer.h include/lm_interp_steps.h include/lm_interp_orders.h include/lm_work_matrix.h tests/lm_p33_cases.h tests/lm_p33_vectors.h
.PHONY: test-lm-p33 sanitize-lm-p33 c64-lm-p33 vice-lm-p33
all: build/test-lm-p33
test: test-lm-p33
sanitize: sanitize-lm-p33
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/lm-p33-reference.snapshots.bin.xz: tools/lm_p33_differential.py tools/lm_p33_reference.c tools/lm_p33_snapshot.c $(LM_P33_SRC) tests/lm_p33_cases.c | build
	python3 tools/lm_p33_differential.py
build/test-lm-p33: $(LM_P33_SRC) tests/lm_p33_cases.c tests/test_lm_p33.c $(LM_P33_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_P33_SRC) tests/lm_p33_cases.c tests/test_lm_p33.c -lm -o $@
test-lm-p33: build/test-lm-p33 build/lm-p33-reference.snapshots.bin.xz
	./build/test-lm-p33
	python3 tools/lm_p33_source.py
sanitize-lm-p33: build/lm-p33-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_P33_SRC) tests/lm_p33_cases.c tests/test_lm_p33.c -lm -o build/test-lm-p33-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p33-sanitize
# Two PRGs, each registering only the routines its cases reach, with the
# image under KERNAL ROM. -fno-inline-functions keeps LTO from copying the
# shared order bodies into the routines (as lm-p30 does).
c64-lm-p33: build/lm-p33-1.prg build/lm-p33-2.prg
build/lm-p33-%.prg: $(LM_P33_SRC) tests/lm_p33_cases.c platform/c64/lm_p33_diagnostic.c $(LM_P33_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_P33_C64_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -fno-inline-functions -Wall -Wextra -Werror -Wl,-Map,build/lm-p33-$*.map $(LM_P33_SRC) tests/lm_p33_cases.c platform/c64/lm_p33_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-p33-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-p33: c64-lm-p33
	python3 tools/vice_run.py build/lm-p33-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-p33-2.prg --cycles 9000000000 --timeout 3500
