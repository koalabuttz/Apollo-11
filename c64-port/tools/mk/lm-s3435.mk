# SPDX-License-Identifier: MIT
# Milestone lm-s3435 (lane E): Luminary P34-35 S34/35.1, S34/35.4, NOVRWRT and LOMAT as transfer routines
LM_S3435_SRC = src/lm_s3435_routines.c src/agc_transfer.c src/lm_interp_orders.c src/lm_interp_steps.c src/matrix_native.c src/guidance_product.c src/guidance_norm.c src/agc_image_ops.c src/agc_hardware_divide.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_S3435_HDR = include/lm_s3435_routines.h include/agc_transfer.h include/lm_interp_steps.h include/lm_interp_orders.h include/matrix_native.h tests/lm_s3435_cases.h tests/lm_s3435_vectors.h
.PHONY: test-lm-s3435 sanitize-lm-s3435
all: build/test-lm-s3435
test: test-lm-s3435
sanitize: sanitize-lm-s3435
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/lm-s3435-reference.snapshots.bin.xz: tools/lm_s3435_differential.py tools/lm_s3435_reference.c tools/lm_s3435_snapshot.c $(LM_S3435_SRC) tests/lm_s3435_cases.c | build
	python3 tools/lm_s3435_differential.py
build/test-lm-s3435: $(LM_S3435_SRC) tests/lm_s3435_cases.c tests/test_lm_s3435.c $(LM_S3435_HDR) tools/mk/lm-s3435.mk | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_S3435_SRC) tests/lm_s3435_cases.c tests/test_lm_s3435.c -lm -o $@
test-lm-s3435: build/test-lm-s3435 build/lm-s3435-reference.snapshots.bin.xz
	./build/test-lm-s3435
	python3 tools/lm_s3435_source.py
sanitize-lm-s3435: build/lm-s3435-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_S3435_SRC) tests/lm_s3435_cases.c tests/test_lm_s3435.c -lm -o build/test-lm-s3435-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-s3435-sanitize
# Two PRGs, each registering only the routines its cases reach, with the
# image under KERNAL ROM.
build/lm-s3435-%.prg: $(LM_S3435_SRC) tests/lm_s3435_cases.c platform/c64/lm_s3435_diagnostic.c $(LM_S3435_HDR) tools/mk/lm-s3435.mk | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_S3435_C64_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-s3435-$*.map $(LM_S3435_SRC) tests/lm_s3435_cases.c platform/c64/lm_s3435_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-s3435-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
# The VICE run as one CI part per group, c64-lm-s3435-<n> and vice-lm-s3435-<n>
# (tools/mk/_ci-split.mk); lm-s3435-c64 builds and lm-s3435-vice runs both.
LM_S3435_C64_PRGS = build/lm-s3435-1.prg build/lm-s3435-2.prg
LM_S3435_VICE = for p in $(2); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
$(eval $(call CI_SPLIT_RULE,lm-s3435,$(LM_S3435_C64_PRGS),2,LM_S3435_VICE))
