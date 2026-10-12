# SPDX-License-Identifier: MIT
# Milestone lanec-ev-time (lane C): EXTENDED_VERBS V55 clock update and ATTCK2, LM and CM.
LANEC_EV_TIME_SRC = src/lanec_ev_time.c tests/lanec_ev_time_cases.c
LANEC_SUITE_SRC += $(LANEC_EV_TIME_SRC)
LANEC_SUITE_HDR += include/lanec_ev_time.h
.PHONY: test-lanec-ev-time sanitize-lanec-ev-time c64-lanec-ev-time vice-lanec-ev-time
test: test-lanec-ev-time
sanitize: sanitize-lanec-ev-time
test-lanec-ev-time: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py ev-time
sanitize-lanec-ev-time: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py ev-time --native build/lanec-native-sanitize
c64-lanec-ev-time: build/lanec-ev-time.prg
build/lanec-ev-time.prg: $(LANEC_CORE) $(LANEC_EV_TIME_SRC) $(LANEC_HDR) include/lanec_ev_time.h include/lanec_ev_internal.h tests/lanec_ev_time_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_ev_time -DLANEC_FROZEN='"lanec_ev_time_c64.h"' -Wl,-Map,build/lanec-ev-time.map \
	  $(LANEC_CORE) $(LANEC_EV_TIME_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-ev-time: build/lanec-ev-time.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-ev-time.prg
