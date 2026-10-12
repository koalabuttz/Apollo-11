# SPDX-License-Identifier: MIT
# Milestone lanec-pin-gaps (lane C): Pinball verb 07 and CHANLOAD, LM and CM.
LANEC_PIN_GAPS_SRC = src/lanec_pin_gaps.c tests/lanec_pin_gaps_cases.c
LANEC_SUITE_SRC += $(LANEC_PIN_GAPS_SRC)
LANEC_SUITE_HDR += include/lanec_pin_gaps.h
.PHONY: test-lanec-pin-gaps sanitize-lanec-pin-gaps c64-lanec-pin-gaps vice-lanec-pin-gaps
test: test-lanec-pin-gaps
sanitize: sanitize-lanec-pin-gaps
test-lanec-pin-gaps: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py pin-gaps
sanitize-lanec-pin-gaps: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py pin-gaps --native build/lanec-native-sanitize
c64-lanec-pin-gaps: build/lanec-pin-gaps.prg
build/lanec-pin-gaps.prg: $(LANEC_CORE) $(LANEC_PIN_GAPS_SRC) $(LANEC_HDR) include/lanec_pin_gaps.h tests/lanec_pin_gaps_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_pin_gaps -DLANEC_FROZEN='"lanec_pin_gaps_c64.h"' -Wl,-Map,build/lanec-pin-gaps.map \
	  $(LANEC_CORE) $(LANEC_PIN_GAPS_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-pin-gaps: build/lanec-pin-gaps.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-pin-gaps.prg
