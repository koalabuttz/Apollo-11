# SPDX-License-Identifier: MIT
# Milestone lanec-di-nvdsp (lane C): DISPLAY_INTERFACE display start-up, LM and CM.
LANEC_DI_NVDSP_SRC = src/lanec_di_nvdsp.c tests/lanec_di_nvdsp_cases.c
LANEC_SUITE_SRC += $(LANEC_DI_NVDSP_SRC)
LANEC_SUITE_HDR += include/lanec_di_nvdsp.h
.PHONY: test-lanec-di-nvdsp sanitize-lanec-di-nvdsp c64-lanec-di-nvdsp vice-lanec-di-nvdsp
test: test-lanec-di-nvdsp
sanitize: sanitize-lanec-di-nvdsp
test-lanec-di-nvdsp: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py di-nvdsp --refusals
sanitize-lanec-di-nvdsp: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py di-nvdsp --refusals --native build/lanec-native-sanitize
c64-lanec-di-nvdsp: build/lanec-di-nvdsp.prg
build/lanec-di-nvdsp.prg: $(LANEC_CORE) $(LANEC_DI_NVDSP_SRC) $(LANEC_HDR) include/lanec_di_nvdsp.h tests/lanec_di_nvdsp_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_di_nvdsp -DLANEC_FROZEN='"lanec_di_nvdsp_c64.h"' -Wl,-Map,build/lanec-di-nvdsp.map \
	  $(LANEC_CORE) $(LANEC_DI_NVDSP_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-di-nvdsp: build/lanec-di-nvdsp.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-di-nvdsp.prg
