# SPDX-License-Identifier: MIT
# Milestone cm-auto-man (lane G): Translate Comanche AUTOMATIC_MANEUVERS (CM RCS attitude hold, rate damping and phase-plane jet timing)
CM_AUTO_MAN_SRC = src/cm_auto_man.c src/agc_image_ops.c src/agc_word.c src/agc_multiword.c src/agc_hardware_divide.c
CM_AUTO_MAN_TEST = tests/test_cm_auto_man.c tests/cm_auto_man_cases.c tests/frozen_words.c
CM_AUTO_MAN_HDR = include/cm_auto_man.h include/agc_image_ops.h tests/cm_auto_man_cases.h tests/cm_auto_man_vectors.h
.PHONY: test-cm-auto-man sanitize-cm-auto-man c64-cm-auto-man vice-cm-auto-man
all: build/test-cm-auto-man
test: test-cm-auto-man
sanitize: sanitize-cm-auto-man
build/test-cm-auto-man: $(CM_AUTO_MAN_SRC) $(CM_AUTO_MAN_TEST) $(CM_AUTO_MAN_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_AUTO_MAN_SRC) $(CM_AUTO_MAN_TEST) -o $@
# All 65536 cases against unchanged Comanche055 in the pinned yaAGC, then the
# frozen rows, then the source map of every emitted original word.
test-cm-auto-man: build/test-cm-auto-man
	./build/test-cm-auto-man
	python3 tools/cm_auto_man_differential.py
	python3 tools/cm_auto_man_source.py
	python3 tools/cm_flight_assembly_comparison.py cm-auto-man AUTOMATIC_MANEUVERS.agc
sanitize-cm-auto-man: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $(CM_AUTO_MAN_SRC) $(CM_AUTO_MAN_TEST) -o build/test-cm-auto-man-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-auto-man-sanitize
	python3 tools/cm_auto_man_differential.py --sanitize
c64-cm-auto-man: build/cm-auto-man.prg
build/cm-auto-man.prg: $(CM_AUTO_MAN_SRC) $(CM_AUTO_MAN_TEST) platform/c64/cm_auto_man_diagnostic.c $(CM_AUTO_MAN_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_AUTO_MAN_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-auto-man.map $(CM_AUTO_MAN_SRC) $(CM_AUTO_MAN_TEST) platform/c64/cm_auto_man_diagnostic.c -o $@
vice-cm-auto-man: build/cm-auto-man.prg
	python3 tools/vice_run.py build/cm-auto-man.prg
