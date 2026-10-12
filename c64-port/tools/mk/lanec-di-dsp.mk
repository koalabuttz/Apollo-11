# SPDX-License-Identifier: MIT
# Milestone lanec-di-dsp (lane C): DISPLAY_INTERFACE display-start entries, LM and CM.
LANEC_DI_DSP_SRC = src/lanec_di_dsp.c tests/lanec_di_dsp_cases.c
LANEC_SUITE_SRC += $(LANEC_DI_DSP_SRC)
LANEC_SUITE_HDR += include/lanec_di_dsp.h
.PHONY: test-lanec-di-dsp sanitize-lanec-di-dsp c64-lanec-di-dsp vice-lanec-di-dsp
test: test-lanec-di-dsp
sanitize: sanitize-lanec-di-dsp
test-lanec-di-dsp: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py di-dsp
sanitize-lanec-di-dsp: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py di-dsp --native build/lanec-native-sanitize
c64-lanec-di-dsp: build/lanec-di-dsp.prg
build/lanec-di-dsp.prg: $(LANEC_CORE) $(LANEC_DI_DSP_SRC) $(LANEC_HDR) include/lanec_di_dsp.h tests/lanec_di_dsp_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_di_dsp -DLANEC_FROZEN='"lanec_di_dsp_c64.h"' -Wl,-Map,build/lanec-di-dsp.map \
	  $(LANEC_CORE) $(LANEC_DI_DSP_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-di-dsp: build/lanec-di-dsp.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-di-dsp.prg
