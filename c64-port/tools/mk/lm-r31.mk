# SPDX-License-Identifier: MIT
# Milestone lm-r31 (lane E): Luminary R31 as transfer routines
LM_R31_SRC = src/lm_r31_routines.c src/lm_work_matrix.c src/agc_transfer.c src/lm_interp_orders.c src/lm_interp_steps.c src/matrix_native.c src/guidance_product.c src/guidance_norm.c src/inverse_trig.c src/rtb_agreement.c src/rtb_vector.c src/rtb_unit.c src/guidance_unit_phased.c src/guidance_length_phased.c src/agc_image_ops.c src/agc_hardware_divide.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_R31_HDR = include/lm_r31_routines.h include/agc_transfer.h include/lm_interp_steps.h include/lm_interp_orders.h include/lm_work_matrix.h include/rtb_unit.h include/rtb_vector.h tests/lm_r31_cases.h tests/lm_r31_vectors.h
.PHONY: test-lm-r31 sanitize-lm-r31 c64-lm-r31 vice-lm-r31-1 vice-lm-r31-2 vice-lm-r31-3
all: build/test-lm-r31
test: test-lm-r31
sanitize: sanitize-lm-r31
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/lm-r31-reference.snapshots.bin.xz: tools/lm_r31_differential.py tools/lm_r31_reference.c tools/lm_r31_snapshot.c $(LM_R31_SRC) tests/lm_r31_cases.c | build
	python3 tools/lm_r31_differential.py
build/test-lm-r31: $(LM_R31_SRC) tests/lm_r31_cases.c tests/test_lm_r31.c $(LM_R31_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_R31_SRC) tests/lm_r31_cases.c tests/test_lm_r31.c -lm -o $@
test-lm-r31: build/test-lm-r31 build/lm-r31-reference.snapshots.bin.xz
	./build/test-lm-r31
	python3 tools/lm_r31_source.py
sanitize-lm-r31: build/lm-r31-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_R31_SRC) tests/lm_r31_cases.c tests/test_lm_r31.c -lm -o build/test-lm-r31-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-r31-sanitize
# Six PRGs, each registering only the routines its cases reach, with the
# image under KERNAL ROM.
c64-lm-r31: build/lm-r31-1.prg build/lm-r31-2.prg build/lm-r31-3.prg build/lm-r31-4.prg build/lm-r31-5.prg build/lm-r31-6.prg
build/lm-r31-%.prg: $(LM_R31_SRC) tests/lm_r31_cases.c platform/c64/lm_r31_diagnostic.c $(LM_R31_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DLM_R31_C64_GROUP=$* -std=c99 -Oz -fno-inline-functions -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-r31-$*.map $(LM_R31_SRC) tests/lm_r31_cases.c platform/c64/lm_r31_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-r31-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
# The VICE run in three pieces, each well under a CI job's ten minutes
# (groups 1-3, 4-5 and 6; tools/ci_target_seconds.json).
vice-lm-r31-1: build/lm-r31-1.prg build/lm-r31-2.prg build/lm-r31-3.prg
	for p in $^; do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
vice-lm-r31-2: build/lm-r31-4.prg build/lm-r31-5.prg
	for p in $^; do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
vice-lm-r31-3: build/lm-r31-6.prg
	python3 tools/vice_run.py build/lm-r31-6.prg --cycles 3000000000 --timeout 1200
