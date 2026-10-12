# SPDX-License-Identifier: MIT
# Milestone cm-rcs-dap (lane G): Translate Comanche RCS-CSM_DIGITAL_AUTOPILOT (CM RCS DAP T5 cycle, needles, hand controller)
CM_RCS_DAP_SRC = src/cm_rcs_dap.c src/agc_image_ops.c src/agc_word.c src/agc_multiword.c src/agc_hardware_divide.c
CM_RCS_DAP_TEST = tests/test_cm_rcs_dap.c tests/cm_rcs_dap_cases.c tests/frozen_words.c
CM_RCS_DAP_HDR = include/cm_rcs_dap.h include/agc_image_ops.h tests/cm_rcs_dap_cases.h tests/cm_rcs_dap_vectors.h
.PHONY: test-cm-rcs-dap sanitize-cm-rcs-dap c64-cm-rcs-dap vice-cm-rcs-dap
all: build/test-cm-rcs-dap
test: test-cm-rcs-dap
sanitize: sanitize-cm-rcs-dap
build/test-cm-rcs-dap: $(CM_RCS_DAP_SRC) $(CM_RCS_DAP_TEST) $(CM_RCS_DAP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_RCS_DAP_SRC) $(CM_RCS_DAP_TEST) -o $@
# All 65536 cases against unchanged Comanche055 in the pinned yaAGC, then the
# frozen rows, then the source map of every emitted original word.
test-cm-rcs-dap: build/test-cm-rcs-dap
	./build/test-cm-rcs-dap
	python3 tools/cm_rcs_dap_differential.py
	python3 tools/cm_rcs_dap_source.py
	python3 tools/cm_flight_assembly_comparison.py cm-rcs-dap RCS-CSM_DIGITAL_AUTOPILOT.agc
sanitize-cm-rcs-dap: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -Wconversion -pedantic -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $(CM_RCS_DAP_SRC) $(CM_RCS_DAP_TEST) -o build/test-cm-rcs-dap-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-rcs-dap-sanitize
	python3 tools/cm_rcs_dap_differential.py --sanitize
c64-cm-rcs-dap: build/cm-rcs-dap.prg
build/cm-rcs-dap.prg: $(CM_RCS_DAP_SRC) $(CM_RCS_DAP_TEST) platform/c64/cm_rcs_dap_diagnostic.c $(CM_RCS_DAP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DCM_RCS_DAP_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-rcs-dap.map $(CM_RCS_DAP_SRC) $(CM_RCS_DAP_TEST) platform/c64/cm_rcs_dap_diagnostic.c -o $@
vice-cm-rcs-dap: build/cm-rcs-dap.prg
	python3 tools/vice_run.py build/cm-rcs-dap.prg
