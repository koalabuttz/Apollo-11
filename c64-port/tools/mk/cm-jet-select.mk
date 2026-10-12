# SPDX-License-Identifier: MIT
# Milestone cm-jet-select (lane G): Translate Comanche JET_SELECTION_LOGIC (CM RCS jet selection T5 task and T6 jet sequencing)
CM_JET_SELECT_SRC = src/cm_jet_select.c src/agc_image_ops.c src/agc_word.c src/agc_multiword.c src/agc_hardware_divide.c
CM_JET_SELECT_TEST = tests/test_cm_jet_select.c tests/cm_jet_select_cases.c tests/frozen_words.c
CM_JET_SELECT_HDR = include/cm_jet_select.h include/agc_image_ops.h tests/cm_jet_select_cases.h tests/cm_jet_select_vectors.h
.PHONY: test-cm-jet-select sanitize-cm-jet-select c64-cm-jet-select vice-cm-jet-select
all: build/test-cm-jet-select
test: test-cm-jet-select
sanitize: sanitize-cm-jet-select
build/test-cm-jet-select: $(CM_JET_SELECT_SRC) $(CM_JET_SELECT_TEST) $(CM_JET_SELECT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_JET_SELECT_SRC) $(CM_JET_SELECT_TEST) -o $@
# All 65536 cases against unchanged Comanche055 in the pinned yaAGC, then the
# frozen rows, then the source map of every emitted original word.
test-cm-jet-select: build/test-cm-jet-select
	./build/test-cm-jet-select
	python3 tools/cm_jet_select_differential.py
	python3 tools/cm_jet_select_source.py
	python3 tools/cm_flight_assembly_comparison.py cm-jet-select JET_SELECTION_LOGIC.agc
sanitize-cm-jet-select: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $(CM_JET_SELECT_SRC) $(CM_JET_SELECT_TEST) -o build/test-cm-jet-select-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-jet-select-sanitize
	python3 tools/cm_jet_select_differential.py --sanitize
c64-cm-jet-select: build/cm-jet-select.prg
build/cm-jet-select.prg: $(CM_JET_SELECT_SRC) $(CM_JET_SELECT_TEST) platform/c64/cm_jet_select_diagnostic.c $(CM_JET_SELECT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_JET_SELECT_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-jet-select.map $(CM_JET_SELECT_SRC) $(CM_JET_SELECT_TEST) platform/c64/cm_jet_select_diagnostic.c -o $@
vice-cm-jet-select: build/cm-jet-select.prg
	python3 tools/vice_run.py build/cm-jet-select.prg
