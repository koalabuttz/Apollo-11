# SPDX-License-Identifier: MIT
# Milestone cm-p21 (lane E): Comanche P21 (GROUND_TRACKING_DETERMINATION_PROGRAM) as transfer routines
CM_P21_SRC = src/cm_p21_routines.c src/agc_transfer.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_word.c src/agc_multiword.c
CM_P21_HDR = include/cm_p21_routines.h include/agc_transfer.h include/cm_interp.h include/interp_mission.h src/cm_interp_names.h src/lm_interp_orders.c src/lm_interp_steps.c tests/cm_p21_cases.h tests/cm_p21_vectors.h
.PHONY: test-cm-p21 sanitize-cm-p21 c64-cm-p21 vice-cm-p21
all: build/test-cm-p21
test: test-cm-p21
sanitize: sanitize-cm-p21
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/cm-p21-reference.snapshots.bin.xz: tools/cm_p21_differential.py tools/cm_p21_reference.c tools/cm_p21_snapshot.c $(CM_P21_SRC) tests/cm_p21_cases.c | build
	python3 tools/cm_p21_differential.py
build/test-cm-p21: $(CM_P21_SRC) tests/cm_p21_cases.c tests/test_cm_p21.c $(CM_P21_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_P21_SRC) tests/cm_p21_cases.c tests/test_cm_p21.c -lm -o $@
test-cm-p21: build/test-cm-p21 build/cm-p21-reference.snapshots.bin.xz
	./build/test-cm-p21
	python3 tools/cm_p21_source.py
sanitize-cm-p21: build/cm-p21-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_P21_SRC) tests/cm_p21_cases.c tests/test_cm_p21.c -lm -o build/test-cm-p21-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-p21-sanitize
# Two PRGs, each registering only the routines its cases reach, with the
# image under KERNAL ROM.
c64-cm-p21: build/cm-p21-1.prg build/cm-p21-2.prg
build/cm-p21-%.prg: $(CM_P21_SRC) tests/cm_p21_cases.c platform/c64/cm_p21_diagnostic.c $(CM_P21_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_P21_C64_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-p21-$*.map $(CM_P21_SRC) tests/cm_p21_cases.c platform/c64/cm_p21_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/cm-p21-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-cm-p21: c64-cm-p21
	python3 tools/vice_run.py build/cm-p21-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/cm-p21-2.prg --cycles 3000000000 --timeout 1200
