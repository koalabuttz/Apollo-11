# SPDX-License-Identifier: MIT
# Milestone cm-entry-dap (lane G): Translate Comanche CM_ENTRY_DIGITAL_AUTOPILOT (CM entry DAP: body rates, attitude, dead zones, roll DAP)
CM_ENTRY_DAP_SRC = src/cm_entry_dap.c src/agc_image_ops.c src/agc_word.c src/agc_multiword.c src/agc_hardware_divide.c
CM_ENTRY_DAP_TEST = tests/test_cm_entry_dap.c tests/cm_entry_dap_cases.c tests/frozen_words.c
CM_ENTRY_DAP_HDR = include/cm_entry_dap.h include/agc_image_ops.h tests/cm_entry_dap_cases.h tests/cm_entry_dap_vectors.h
.PHONY: test-cm-entry-dap sanitize-cm-entry-dap c64-cm-entry-dap vice-cm-entry-dap
all: build/test-cm-entry-dap
test: test-cm-entry-dap
sanitize: sanitize-cm-entry-dap
build/test-cm-entry-dap: $(CM_ENTRY_DAP_SRC) $(CM_ENTRY_DAP_TEST) $(CM_ENTRY_DAP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_ENTRY_DAP_SRC) $(CM_ENTRY_DAP_TEST) -o $@
# All 65536 cases against unchanged Comanche055 in the pinned yaAGC, then the
# frozen rows, then the source map of every emitted original word.
test-cm-entry-dap: build/test-cm-entry-dap
	./build/test-cm-entry-dap
	python3 tools/cm_entry_dap_differential.py
	python3 tools/cm_entry_dap_source.py
	python3 tools/cm_flight_assembly_comparison.py cm-entry-dap CM_ENTRY_DIGITAL_AUTOPILOT.agc
sanitize-cm-entry-dap: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $(CM_ENTRY_DAP_SRC) $(CM_ENTRY_DAP_TEST) -o build/test-cm-entry-dap-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-entry-dap-sanitize
	python3 tools/cm_entry_dap_differential.py --sanitize
c64-cm-entry-dap: build/cm-entry-dap.prg
build/cm-entry-dap.prg: $(CM_ENTRY_DAP_SRC) $(CM_ENTRY_DAP_TEST) platform/c64/cm_entry_dap_diagnostic.c $(CM_ENTRY_DAP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_ENTRY_DAP_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-entry-dap.map $(CM_ENTRY_DAP_SRC) $(CM_ENTRY_DAP_TEST) platform/c64/cm_entry_dap_diagnostic.c -o $@
vice-cm-entry-dap: build/cm-entry-dap.prg
	python3 tools/vice_run.py build/cm-entry-dap.prg
