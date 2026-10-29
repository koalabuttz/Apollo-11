# SPDX-License-Identifier: MIT
# Milestone lm-s3435b (lane E): Luminary P34-35 S34/35.3, GOINT and INTINT as transfer routines
LM_S3435B_SRC = src/lm_s3435b_routines.c src/lm_s3435_routines.c src/agc_transfer.c src/lm_interp_orders.c src/lm_interp_steps.c src/matrix_native.c src/guidance_product.c src/guidance_norm.c src/agc_image_ops.c src/agc_hardware_divide.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_S3435B_HDR = include/lm_s3435b_routines.h include/lm_s3435_routines.h include/agc_transfer.h include/lm_interp_steps.h include/lm_interp_orders.h include/matrix_native.h tests/lm_s3435b_cases.h tests/lm_s3435b_vectors.h
.PHONY: test-lm-s3435b sanitize-lm-s3435b c64-lm-s3435b vice-lm-s3435b
all: build/test-lm-s3435b
test: test-lm-s3435b
sanitize: sanitize-lm-s3435b
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/lm-s3435b-reference.snapshots.bin.xz: tools/lm_s3435b_differential.py tools/lm_s3435b_reference.c tools/lm_s3435b_snapshot.c $(LM_S3435B_SRC) tests/lm_s3435b_cases.c | build
	python3 tools/lm_s3435b_differential.py
build/test-lm-s3435b: $(LM_S3435B_SRC) tests/lm_s3435b_cases.c tests/test_lm_s3435b.c $(LM_S3435B_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_S3435B_SRC) tests/lm_s3435b_cases.c tests/test_lm_s3435b.c -lm -o $@
test-lm-s3435b: build/test-lm-s3435b build/lm-s3435b-reference.snapshots.bin.xz
	./build/test-lm-s3435b
	python3 tools/lm_s3435b_source.py
sanitize-lm-s3435b: build/lm-s3435b-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_S3435B_SRC) tests/lm_s3435b_cases.c tests/test_lm_s3435b.c -lm -o build/test-lm-s3435b-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-s3435b-sanitize
# Three PRGs, each registering only the routines its cases reach, with the
# image under KERNAL ROM: groups 1 and 2 link none of lm-s3435, group 3 its
# NOVRWRT (LM_S3435_C64_GROUP=3).
LM_S3435B_C64_1 = $(filter-out src/lm_s3435_routines.c,$(LM_S3435B_SRC))
LM_S3435B_C64_2 = $(LM_S3435B_C64_1)
LM_S3435B_C64_3 = $(LM_S3435B_SRC) -DLM_S3435_C64_GROUP=3
c64-lm-s3435b: build/lm-s3435b-1.prg build/lm-s3435b-2.prg build/lm-s3435b-3.prg
build/lm-s3435b-%.prg: $(LM_S3435B_SRC) tests/lm_s3435b_cases.c platform/c64/lm_s3435b_diagnostic.c $(LM_S3435B_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_S3435B_C64_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-s3435b-$*.map $(LM_S3435B_C64_$*) tests/lm_s3435b_cases.c platform/c64/lm_s3435b_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-s3435b-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-s3435b: c64-lm-s3435b
	python3 tools/vice_run.py build/lm-s3435b-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-s3435b-2.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-s3435b-3.prg --cycles 3000000000 --timeout 1200
