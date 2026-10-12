# SPDX-License-Identifier: MIT
# Milestone lm-midgim (lane E): Translate Luminary MIDGIM (GET+MGA, GET.LVC) and SELECTMU (P34-35)
LM_MIDGIM_SRC = src/lm_midgim.c src/inverse_trig.c src/guidance_norm.c src/guidance_product.c src/matrix_native.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/rtb_agreement.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_MIDGIM_TARGET = tests/lm_midgim_target.c tests/lm_midgim_phase_target.c tests/lm_midgim_cases.c tests/lm_midgim_debris.c tests/frozen_words.c
LM_MIDGIM_C64 = tests/lm_midgim_phase_target.c tests/lm_midgim_debris.c tests/frozen_words.c
LM_MIDGIM_HDR = include/lm_midgim.h include/inverse_trig.h include/matrix_native.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h tests/lm_midgim_target.h tests/lm_midgim_vectors.h tests/lm_midgim_phase_target.h tests/lm_midgim_g1_vectors.h tests/lm_midgim_g2_vectors.h tests/lm_midgim_g3_vectors.h tests/lm_midgim_cases.h
.PHONY: test-lm-midgim sanitize-lm-midgim c64-lm-midgim vice-lm-midgim
all: build/test-lm-midgim
test: test-lm-midgim
sanitize: sanitize-lm-midgim
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native producer and against the pinned stream identity.
build/lm-midgim-reference.snapshots.bin.xz: tools/lm_midgim_differential.py tools/lm_midgim_reference.c $(LM_MIDGIM_SRC) tests/lm_midgim_cases.c tests/lm_midgim_debris.c | build
	python3 tools/lm_midgim_differential.py
build/test-lm-midgim: $(LM_MIDGIM_SRC) $(LM_MIDGIM_TARGET) tests/test_lm_midgim.c $(LM_MIDGIM_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_MIDGIM_SRC) $(LM_MIDGIM_TARGET) tests/test_lm_midgim.c -lm -o $@
test-lm-midgim: build/test-lm-midgim build/lm-midgim-reference.snapshots.bin.xz
	./build/test-lm-midgim
	python3 tools/lm_midgim_host.py --cc "$(CC)"
	python3 tools/lm_midgim_source.py
sanitize-lm-midgim: build/lm-midgim-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_MIDGIM_SRC) $(LM_MIDGIM_TARGET) tests/test_lm_midgim.c -lm -o build/test-lm-midgim-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-midgim-sanitize
	python3 tools/lm_midgim_host.py --cc "$(CC)" --sanitize
# Three PRGs, one per phase group, with the two images under KERNAL ROM.
c64-lm-midgim: build/lm-midgim-1.prg build/lm-midgim-2.prg build/lm-midgim-3.prg
build/lm-midgim-%.prg: $(LM_MIDGIM_SRC) $(LM_MIDGIM_C64) platform/c64/lm_midgim_diagnostic.c $(LM_MIDGIM_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_MIDGIM_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-midgim-$*.map $(LM_MIDGIM_SRC) $(LM_MIDGIM_C64) platform/c64/lm_midgim_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-midgim-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-midgim: c64-lm-midgim
	python3 tools/vice_run.py build/lm-midgim-1.prg
	python3 tools/vice_run.py build/lm-midgim-2.prg
	python3 tools/vice_run.py build/lm-midgim-3.prg
