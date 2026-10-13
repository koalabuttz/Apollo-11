# SPDX-License-Identifier: MIT
# Milestone lm-p76 (lane E): Luminary P76 as transfer routines
LM_P76_SRC = src/lm_p76_routines.c src/lm_work_matrix.c src/agc_transfer.c src/lm_interp_orders.c src/lm_interp_steps.c src/matrix_native.c src/guidance_product.c src/guidance_norm.c src/inverse_trig.c src/rtb_agreement.c src/agc_image_ops.c src/agc_hardware_divide.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_P76_HDR = include/lm_p76_routines.h include/agc_transfer.h include/lm_interp_steps.h include/lm_interp_orders.h include/lm_work_matrix.h tests/lm_p76_cases.h tests/lm_p76_vectors.h
.PHONY: test-lm-p76 sanitize-lm-p76 c64-lm-p76 vice-lm-p76
all: build/test-lm-p76
test: test-lm-p76
sanitize: sanitize-lm-p76
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/lm-p76-reference.snapshots.bin.xz: tools/lm_p76_differential.py tools/lm_p76_reference.c tools/lm_p76_snapshot.c $(LM_P76_SRC) tests/lm_p76_cases.c | build
	python3 tools/lm_p76_differential.py
build/test-lm-p76: $(LM_P76_SRC) tests/lm_p76_cases.c tests/test_lm_p76.c $(LM_P76_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_P76_SRC) tests/lm_p76_cases.c tests/test_lm_p76.c -lm -o $@
test-lm-p76: build/test-lm-p76 build/lm-p76-reference.snapshots.bin.xz
	./build/test-lm-p76
	python3 tools/lm_p76_source.py
sanitize-lm-p76: build/lm-p76-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_P76_SRC) tests/lm_p76_cases.c tests/test_lm_p76.c -lm -o build/test-lm-p76-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p76-sanitize
# Two PRGs, each registering only the routines its cases reach, with the
# image under KERNAL ROM.
c64-lm-p76: build/lm-p76-1.prg build/lm-p76-2.prg
build/lm-p76-%.prg: $(LM_P76_SRC) tests/lm_p76_cases.c platform/c64/lm_p76_diagnostic.c $(LM_P76_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_P76_C64_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-p76-$*.map $(LM_P76_SRC) tests/lm_p76_cases.c platform/c64/lm_p76_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-p76-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-p76: c64-lm-p76
	python3 tools/vice_run.py build/lm-p76-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/lm-p76-2.prg --cycles 3000000000 --timeout 1200
