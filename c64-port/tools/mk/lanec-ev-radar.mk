# SPDX-License-Identifier: MIT
# Milestone lanec-ev-radar (lane C): EXTENDED_VERBS radar test verbs, LM.
LANEC_EV_RADAR_SRC = src/lanec_ev_radar.c tests/lanec_ev_radar_cases.c
LANEC_SUITE_SRC += $(LANEC_EV_RADAR_SRC)
LANEC_SUITE_HDR += include/lanec_ev_radar.h
.PHONY: test-lanec-ev-radar sanitize-lanec-ev-radar c64-lanec-ev-radar vice-lanec-ev-radar
test: test-lanec-ev-radar
sanitize: sanitize-lanec-ev-radar
test-lanec-ev-radar: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py ev-radar
sanitize-lanec-ev-radar: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py ev-radar --native build/lanec-native-sanitize
c64-lanec-ev-radar: build/lanec-ev-radar.prg
build/lanec-ev-radar.prg: $(LANEC_CORE) $(LANEC_EV_RADAR_SRC) $(LANEC_HDR) include/lanec_ev_radar.h include/lanec_ev_internal.h tests/lanec_ev_radar_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_ev_radar -DLANEC_FROZEN='"lanec_ev_radar_c64.h"' -Wl,-Map,build/lanec-ev-radar.map \
	  $(LANEC_CORE) $(LANEC_EV_RADAR_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-ev-radar: build/lanec-ev-radar.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-ev-radar.prg
