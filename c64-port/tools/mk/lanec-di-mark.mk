# SPDX-License-Identifier: MIT
# Milestone lanec-di-mark (lane C): DISPLAY_INTERFACE mark displays, LM and CM.
LANEC_DI_MARK_SRC = src/lanec_di_mark.c tests/lanec_di_mark_cases.c
LANEC_SUITE_SRC += $(LANEC_DI_MARK_SRC)
LANEC_SUITE_HDR += include/lanec_di_mark.h include/lanec_di_internal.h
.PHONY: test-lanec-di-mark sanitize-lanec-di-mark c64-lanec-di-mark vice-lanec-di-mark
test: test-lanec-di-mark
sanitize: sanitize-lanec-di-mark
test-lanec-di-mark: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py di-mark
sanitize-lanec-di-mark: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py di-mark --native build/lanec-native-sanitize
c64-lanec-di-mark: build/lanec-di-mark.prg
build/lanec-di-mark.prg: $(LANEC_CORE) $(LANEC_DI_MARK_SRC) src/lanec_di_dsp.c $(LANEC_HDR) include/lanec_di_mark.h include/lanec_di_internal.h tests/lanec_di_mark_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_di_mark -DLANEC_FROZEN='"lanec_di_mark_c64.h"' -Wl,-Map,build/lanec-di-mark.map \
	  $(LANEC_CORE) $(LANEC_DI_MARK_SRC) src/lanec_di_dsp.c tools/lanec/c64_diag.c -o $@
vice-lanec-di-mark: build/lanec-di-mark.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-di-mark.prg
