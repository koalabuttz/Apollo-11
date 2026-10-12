# SPDX-License-Identifier: MIT
# Milestone cm-p76 (lane E): Comanche P76 as transfer routines
CM_P76_SRC = src/cm_p76_routines.c src/agc_transfer.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_word.c src/agc_multiword.c
CM_P76_HDR = include/cm_p76_routines.h include/agc_transfer.h include/cm_interp.h include/interp_mission.h src/cm_interp_names.h src/lm_interp_orders.c src/lm_interp_steps.c tests/cm_p76_cases.h tests/cm_p76_vectors.h
.PHONY: test-cm-p76 sanitize-cm-p76 c64-cm-p76 vice-cm-p76
all: build/test-cm-p76
test: test-cm-p76
sanitize: sanitize-cm-p76
# Oracle stream: unchanged Luminary099 in the pinned yaAGC, checked row by row
# against the native trampoline runs and against the pinned stream identity.
build/cm-p76-reference.snapshots.bin.xz: tools/cm_p76_differential.py tools/cm_p76_reference.c tools/cm_p76_snapshot.c $(CM_P76_SRC) tests/cm_p76_cases.c | build
	python3 tools/cm_p76_differential.py
build/test-cm-p76: $(CM_P76_SRC) tests/cm_p76_cases.c tests/test_cm_p76.c $(CM_P76_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_P76_SRC) tests/cm_p76_cases.c tests/test_cm_p76.c -lm -o $@
test-cm-p76: build/test-cm-p76 build/cm-p76-reference.snapshots.bin.xz
	./build/test-cm-p76
	python3 tools/cm_p76_source.py
sanitize-cm-p76: build/cm-p76-reference.snapshots.bin.xz | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_P76_SRC) tests/cm_p76_cases.c tests/test_cm_p76.c -lm -o build/test-cm-p76-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-p76-sanitize
# Two PRGs, each registering only the routines its cases reach, with the
# image under KERNAL ROM.
c64-cm-p76: build/cm-p76-1.prg build/cm-p76-2.prg
build/cm-p76-%.prg: $(CM_P76_SRC) tests/cm_p76_cases.c platform/c64/cm_p76_diagnostic.c $(CM_P76_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_P76_C64_GROUP=$* -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-p76-$*.map $(CM_P76_SRC) tests/cm_p76_cases.c platform/c64/cm_p76_diagnostic.c -o $@
	python3 -c "import re,sys;m=open('build/cm-p76-$*.map').read();e=max(int(x,16) for x in re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+__heap_start',m,re.M));sys.exit(0 if e<=0xc000 else 'image and BSS reach the mailbox page: '+hex(e))"
vice-cm-p76: c64-cm-p76
	python3 tools/vice_run.py build/cm-p76-1.prg --cycles 3000000000 --timeout 1200
	python3 tools/vice_run.py build/cm-p76-2.prg --cycles 3000000000 --timeout 1200
