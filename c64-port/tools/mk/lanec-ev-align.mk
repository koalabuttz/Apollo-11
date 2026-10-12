# SPDX-License-Identifier: MIT
# Milestone lanec-ev-align (lane C): EXTENDED_VERBS alignment and radar-designate bodies, LM and CM.
LANEC_EV_ALIGN_SRC = src/lanec_ev_align.c tests/lanec_ev_align_cases.c
LANEC_SUITE_SRC += $(LANEC_EV_ALIGN_SRC)
LANEC_SUITE_HDR += include/lanec_ev_align.h
.PHONY: test-lanec-ev-align sanitize-lanec-ev-align c64-lanec-ev-align vice-lanec-ev-align
test: test-lanec-ev-align
sanitize: sanitize-lanec-ev-align
test-lanec-ev-align: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py ev-align
sanitize-lanec-ev-align: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py ev-align --native build/lanec-native-sanitize
c64-lanec-ev-align: build/lanec-ev-align.prg
build/lanec-ev-align.prg: $(LANEC_CORE) $(LANEC_EV_ALIGN_SRC) $(LANEC_HDR) include/lanec_ev_align.h include/lanec_ev_internal.h tests/lanec_ev_align_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_ev_align -DLANEC_FROZEN='"lanec_ev_align_c64.h"' -Wl,-Map,build/lanec-ev-align.map \
	  $(LANEC_CORE) $(LANEC_EV_ALIGN_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-ev-align: build/lanec-ev-align.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-ev-align.prg
