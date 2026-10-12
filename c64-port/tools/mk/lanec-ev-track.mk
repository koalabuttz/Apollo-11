# SPDX-License-Identifier: MIT
# Milestone lanec-ev-track (lane C): EXTENDED_VERBS tracking and state-vector verb glue, LM and CM.
LANEC_EV_TRACK_SRC = src/lanec_ev_track.c tests/lanec_ev_track_cases.c
LANEC_SUITE_SRC += $(LANEC_EV_TRACK_SRC)
LANEC_SUITE_HDR += include/lanec_ev_track.h
.PHONY: test-lanec-ev-track sanitize-lanec-ev-track c64-lanec-ev-track vice-lanec-ev-track
test: test-lanec-ev-track
sanitize: sanitize-lanec-ev-track
test-lanec-ev-track: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py ev-track
sanitize-lanec-ev-track: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py ev-track --native build/lanec-native-sanitize
c64-lanec-ev-track: build/lanec-ev-track.prg
build/lanec-ev-track.prg: $(LANEC_CORE) $(LANEC_EV_TRACK_SRC) $(LANEC_HDR) include/lanec_ev_track.h include/lanec_ev_internal.h tests/lanec_ev_track_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_ev_track -DLANEC_FROZEN='"lanec_ev_track_c64.h"' -Wl,-Map,build/lanec-ev-track.map \
	  $(LANEC_CORE) $(LANEC_EV_TRACK_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-ev-track: build/lanec-ev-track.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-ev-track.prg
