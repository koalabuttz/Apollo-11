# SPDX-License-Identifier: MIT
# Milestone lm-s3435c (lane E): Luminary P34-35 S34/35.2's integration head and LOMAT tail and S34/35.5 as transfer routines
LM_S3435C_SRC = src/lm_s3435c_routines.c src/lm_s3435b_routines.c src/lm_s3435_routines.c src/lm_p34_routines.c src/lm_vn1645.c src/lm_midgim.c src/inverse_trig.c src/rtb_agreement.c src/lm_interbank_routines.c src/lm_interbank.c src/lm_work_matrix.c src/agc_transfer.c src/lm_interp_orders.c src/lm_interp_steps.c src/matrix_native.c src/guidance_product.c src/guidance_norm.c src/agc_image_ops.c src/agc_hardware_divide.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_S3435C_HDR = include/lm_s3435c_routines.h include/lm_s3435b_routines.h include/lm_s3435_routines.h include/lm_p34_routines.h include/lm_work_matrix.h include/agc_transfer.h include/lm_interp_steps.h include/lm_interp_orders.h include/matrix_native.h tests/lm_s3435c_cases.h tests/lm_s3435c_vectors.h
.PHONY: test-lm-s3435c sanitize-lm-s3435c c64-lm-s3435c vice-lm-s3435c
all: build/test-lm-s3435c
test: test-lm-s3435c
sanitize: sanitize-lm-s3435c
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/lm-s3435c-reference.snapshots.bin.xz: tools/lm_s3435c_differential.py tools/lm_s3435c_reference.c tools/lm_s3435c_snapshot.c $(LM_S3435C_SRC) tests/lm_s3435c_cases.c | build
	python3 tools/lm_s3435c_differential.py
build/test-lm-s3435c: $(LM_S3435C_SRC) tests/lm_s3435c_cases.c tests/test_lm_s3435c.c $(LM_S3435C_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_S3435C_SRC) tests/lm_s3435c_cases.c tests/test_lm_s3435c.c -lm -o $@
test-lm-s3435c: build/test-lm-s3435c build/lm-s3435c-reference.snapshots.bin.xz
	./build/test-lm-s3435c
	python3 tools/lm_s3435c_source.py
sanitize-lm-s3435c: build/lm-s3435c-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_S3435C_SRC) tests/lm_s3435c_cases.c tests/test_lm_s3435c.c -lm -o build/test-lm-s3435c-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-s3435c-sanitize
# Three PRGs, each registering only the routines its cases reach, with the
# image under KERNAL ROM: group 1 links lm-s3435b's INTINT
# (LM_S3435B_C64_GROUP=2); link-time optimization drops the tables a group
# never registers.
LM_S3435C_C64_FLAGS_1 = -DLM_S3435B_C64_GROUP=2
LM_S3435C_C64_FLAGS_2 =
LM_S3435C_C64_FLAGS_3 =
c64-lm-s3435c: build/lm-s3435c-1.prg build/lm-s3435c-2.prg build/lm-s3435c-3.prg
build/lm-s3435c-%.prg: $(LM_S3435C_SRC) tests/lm_s3435c_cases.c platform/c64/lm_s3435c_diagnostic.c $(LM_S3435C_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_S3435C_C64_GROUP=$* $(LM_S3435C_C64_FLAGS_$*) -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-s3435c-$*.map $(LM_S3435C_SRC) tests/lm_s3435c_cases.c platform/c64/lm_s3435c_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-s3435c-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-s3435c: c64-lm-s3435c
	python3 tools/vice_run.py build/lm-s3435c-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-s3435c-2.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-s3435c-3.prg --cycles 3000000000 --timeout 1200
