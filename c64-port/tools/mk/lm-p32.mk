# SPDX-License-Identifier: MIT
# Milestone lm-p32 (lane E): Luminary P32/P72 with ALMXITA and ALMXIT as transfer routines
LM_P32_SRC = src/lm_p32_routines.c src/agc_transfer.c src/lm_interp_orders.c src/lm_interp_steps.c src/matrix_native.c src/guidance_product.c src/guidance_norm.c src/inverse_trig.c src/rtb_agreement.c src/agc_image_ops.c src/agc_hardware_divide.c src/fixed_data.c src/agc_word.c src/agc_multiword.c
LM_P32_HDR = include/lm_p32_routines.h include/agc_transfer.h include/lm_interp_steps.h include/lm_interp_orders.h tests/lm_p32_cases.h tests/lm_p32_vectors.h
.PHONY: test-lm-p32 sanitize-lm-p32 c64-lm-p32 vice-lm-p32
all: build/test-lm-p32
test: test-lm-p32
sanitize: sanitize-lm-p32
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/lm-p32-reference.snapshots.bin.xz: tools/lm_p32_differential.py tools/lm_p32_reference.c tools/lm_p32_snapshot.c $(LM_P32_SRC) tests/lm_p32_cases.c | build
	python3 tools/lm_p32_differential.py
build/test-lm-p32: $(LM_P32_SRC) tests/lm_p32_cases.c tests/test_lm_p32.c $(LM_P32_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_P32_SRC) tests/lm_p32_cases.c tests/test_lm_p32.c -lm -o $@
test-lm-p32: build/test-lm-p32 build/lm-p32-reference.snapshots.bin.xz
	./build/test-lm-p32
	python3 tools/lm_p32_source.py
sanitize-lm-p32: build/lm-p32-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_P32_SRC) tests/lm_p32_cases.c tests/test_lm_p32.c -lm -o build/test-lm-p32-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-p32-sanitize
# One PRG registering every routine, with the image under KERNAL ROM.
# -fno-inline-functions keeps LTO from copying the shared order bodies into
# the routines (as lm-p30 does).
c64-lm-p32: build/lm-p32-1.prg
build/lm-p32-%.prg: $(LM_P32_SRC) tests/lm_p32_cases.c platform/c64/lm_p32_diagnostic.c $(LM_P32_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_LM_ONLY -DLM_P32_C64_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -fno-inline-functions -Wall -Wextra -Werror -Wl,-Map,build/lm-p32-$*.map $(LM_P32_SRC) tests/lm_p32_cases.c platform/c64/lm_p32_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/lm-p32-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-lm-p32: c64-lm-p32
	python3 tools/vice_run.py build/lm-p32-1.prg --cycles 9000000000 --timeout 3500
