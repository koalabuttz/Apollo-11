# SPDX-License-Identifier: MIT
# Milestone lm-vn1645 (lane E): Translate Luminary VN1645's computation head and P3XORP7X (P34-35)
LM_VN1645_SRC = src/lm_vn1645.c src/lm_midgim.c src/inverse_trig.c src/guidance_norm.c src/guidance_product.c src/matrix_native.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/rtb_agreement.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_VN1645_TARGET = tests/lm_vn1645_target.c tests/lm_vn1645_phase_target.c tests/lm_vn1645_cases.c tests/lm_vn1645_debris.c tests/frozen_words.c
LM_VN1645_C64 = tests/lm_vn1645_phase_target.c tests/lm_vn1645_debris.c tests/frozen_words.c
LM_VN1645_HDR = include/lm_vn1645.h include/lm_midgim.h include/inverse_trig.h include/matrix_native.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h tests/lm_vn1645_target.h tests/lm_vn1645_vectors.h tests/lm_vn1645_phase_target.h tests/lm_vn1645_g1_vectors.h tests/lm_vn1645_g2_vectors.h tests/lm_vn1645_cases.h
.PHONY: test-lm-vn1645 sanitize-lm-vn1645 c64-lm-vn1645 vice-lm-vn1645
all: build/test-lm-vn1645
test: test-lm-vn1645
sanitize: sanitize-lm-vn1645
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native producer and against the pinned stream identity.
build/lm-vn1645-reference.snapshots.bin.xz: tools/lm_vn1645_differential.py tools/lm_vn1645_reference.c $(LM_VN1645_SRC) tests/lm_vn1645_cases.c tests/lm_vn1645_debris.c | build
	python3 tools/lm_vn1645_differential.py
build/test-lm-vn1645: $(LM_VN1645_SRC) $(LM_VN1645_TARGET) tests/test_lm_vn1645.c $(LM_VN1645_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_VN1645_SRC) $(LM_VN1645_TARGET) tests/test_lm_vn1645.c -lm -o $@
test-lm-vn1645: build/test-lm-vn1645 build/lm-vn1645-reference.snapshots.bin.xz
	./build/test-lm-vn1645
	python3 tools/lm_vn1645_host.py --cc "$(CC)"
	python3 tools/lm_vn1645_source.py
sanitize-lm-vn1645: build/lm-vn1645-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_VN1645_SRC) $(LM_VN1645_TARGET) tests/test_lm_vn1645.c -lm -o build/test-lm-vn1645-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-vn1645-sanitize
	python3 tools/lm_vn1645_host.py --cc "$(CC)" --sanitize
# Two PRGs, one per phase group, with the two images under KERNAL ROM.
c64-lm-vn1645: build/lm-vn1645-1.prg build/lm-vn1645-2.prg
build/lm-vn1645-%.prg: $(LM_VN1645_SRC) $(LM_VN1645_C64) platform/c64/lm_vn1645_diagnostic.c $(LM_VN1645_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_VN1645_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-vn1645-$*.map $(LM_VN1645_SRC) $(LM_VN1645_C64) platform/c64/lm_vn1645_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-vn1645-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-vn1645: c64-lm-vn1645
	python3 tools/vice_run.py build/lm-vn1645-1.prg
	python3 tools/vice_run.py build/lm-vn1645-2.prg
