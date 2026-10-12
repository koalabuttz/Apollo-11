# SPDX-License-Identifier: MIT
# Milestone lm-p30 (lane E): Luminary P30 and S30.1 as transfer routines
LM_P30_SRC = src/lm_p30_routines.c src/lm_work_matrix.c src/agc_transfer.c src/lm_interp_orders.c src/lm_interp_steps.c src/matrix_native.c src/guidance_product.c src/guidance_norm.c src/inverse_trig.c src/rtb_agreement.c src/agc_image_ops.c src/agc_hardware_divide.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_P30_HDR = include/lm_p30_routines.h include/agc_transfer.h include/lm_interp_steps.h include/lm_interp_orders.h include/lm_work_matrix.h tests/lm_p30_cases.h tests/lm_p30_vectors.h
.PHONY: test-lm-p30 sanitize-lm-p30 c64-lm-p30 vice-lm-p30
all: build/test-lm-p30
test: test-lm-p30
sanitize: sanitize-lm-p30
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/lm-p30-reference.snapshots.bin.xz: tools/lm_p30_differential.py tools/lm_p30_reference.c tools/lm_p30_snapshot.c $(LM_P30_SRC) tests/lm_p30_cases.c | build
	python3 tools/lm_p30_differential.py
build/test-lm-p30: $(LM_P30_SRC) tests/lm_p30_cases.c tests/test_lm_p30.c $(LM_P30_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_P30_SRC) tests/lm_p30_cases.c tests/test_lm_p30.c -lm -o $@
test-lm-p30: build/test-lm-p30 build/lm-p30-reference.snapshots.bin.xz
	./build/test-lm-p30
	python3 tools/lm_p30_source.py
sanitize-lm-p30: build/lm-p30-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_P30_SRC) tests/lm_p30_cases.c tests/test_lm_p30.c -lm -o build/test-lm-p30-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p30-sanitize
# Two PRGs, each registering only the routines its cases reach, with the
# image under KERNAL ROM. -fno-inline-functions keeps LTO from copying the
# shared order bodies into S30.1's LEMPREC return (group 1 fits under the
# mailbox page only without the copies).
c64-lm-p30: build/lm-p30-1.prg build/lm-p30-2.prg
build/lm-p30-%.prg: $(LM_P30_SRC) tests/lm_p30_cases.c platform/c64/lm_p30_diagnostic.c $(LM_P30_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_P30_C64_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -fno-inline-functions -Wall -Wextra -Werror -Wl,-Map,build/lm-p30-$*.map $(LM_P30_SRC) tests/lm_p30_cases.c platform/c64/lm_p30_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-p30-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-p30: c64-lm-p30
	python3 tools/vice_run.py build/lm-p30-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-p30-2.prg --cycles 3000000000 --timeout 1200
