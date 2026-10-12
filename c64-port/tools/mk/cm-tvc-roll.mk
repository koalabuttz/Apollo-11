# SPDX-License-Identifier: MIT
# Milestone cm-tvc-roll (lane G): Translate Comanche TVCROLLDAP (CM roll DAP during TVC: phase plane, firing time, roll jets)
CM_TVC_ROLL_SRC = src/cm_tvc_roll.c src/agc_image_ops.c src/agc_word.c src/agc_multiword.c src/agc_hardware_divide.c
CM_TVC_ROLL_TEST = tests/test_cm_tvc_roll.c tests/cm_tvc_roll_cases.c tests/frozen_words.c
CM_TVC_ROLL_HDR = include/cm_tvc_roll.h include/agc_image_ops.h tests/cm_tvc_roll_cases.h tests/cm_tvc_roll_vectors.h
.PHONY: test-cm-tvc-roll sanitize-cm-tvc-roll c64-cm-tvc-roll vice-cm-tvc-roll
all: build/test-cm-tvc-roll
test: test-cm-tvc-roll
sanitize: sanitize-cm-tvc-roll
build/test-cm-tvc-roll: $(CM_TVC_ROLL_SRC) $(CM_TVC_ROLL_TEST) $(CM_TVC_ROLL_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TVC_ROLL_SRC) $(CM_TVC_ROLL_TEST) -o $@
# All 65536 cases against unchanged Comanche055 in the pinned yaAGC, then the
# frozen rows, then the source map of every emitted original word.
test-cm-tvc-roll: build/test-cm-tvc-roll
	./build/test-cm-tvc-roll
	python3 tools/cm_tvc_roll_differential.py
	python3 tools/cm_tvc_roll_source.py
	python3 tools/cm_flight_assembly_comparison.py cm-tvc-roll TVCROLLDAP.agc
sanitize-cm-tvc-roll: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $(CM_TVC_ROLL_SRC) $(CM_TVC_ROLL_TEST) -o build/test-cm-tvc-roll-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tvc-roll-sanitize
	python3 tools/cm_tvc_roll_differential.py --sanitize
c64-cm-tvc-roll: build/cm-tvc-roll.prg
build/cm-tvc-roll.prg: $(CM_TVC_ROLL_SRC) $(CM_TVC_ROLL_TEST) platform/c64/cm_tvc_roll_diagnostic.c $(CM_TVC_ROLL_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_TVC_ROLL_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-tvc-roll.map $(CM_TVC_ROLL_SRC) $(CM_TVC_ROLL_TEST) platform/c64/cm_tvc_roll_diagnostic.c -o $@
vice-cm-tvc-roll: build/cm-tvc-roll.prg
	python3 tools/vice_run.py build/cm-tvc-roll.prg
