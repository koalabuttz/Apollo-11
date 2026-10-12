# SPDX-License-Identifier: MIT
# Milestone lanec-ev-short (lane C): EXTENDED_VERBS short verb bodies, LM and CM.
LANEC_EV_SHORT_SRC = src/lanec_ev_short.c tests/lanec_ev_short_cases.c
LANEC_SUITE_SRC += $(LANEC_EV_SHORT_SRC)
LANEC_SUITE_HDR += include/lanec_ev_short.h include/lanec_ev_internal.h
.PHONY: test-lanec-ev-short sanitize-lanec-ev-short c64-lanec-ev-short vice-lanec-ev-short
test: test-lanec-ev-short
sanitize: sanitize-lanec-ev-short
test-lanec-ev-short: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py ev-short
sanitize-lanec-ev-short: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py ev-short --native build/lanec-native-sanitize
c64-lanec-ev-short: build/lanec-ev-short.prg
build/lanec-ev-short.prg: $(LANEC_CORE) $(LANEC_EV_SHORT_SRC) $(LANEC_HDR) include/lanec_ev_short.h include/lanec_ev_internal.h tests/lanec_ev_short_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_ev_short -DLANEC_FROZEN='"lanec_ev_short_c64.h"' -Wl,-Map,build/lanec-ev-short.map \
	  $(LANEC_CORE) $(LANEC_EV_SHORT_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-ev-short: build/lanec-ev-short.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-ev-short.prg
