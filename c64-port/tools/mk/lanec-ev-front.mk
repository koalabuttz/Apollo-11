# SPDX-License-Identifier: MIT
# Milestone lanec-ev-front (lane C): EXTENDED_VERBS front end, LM and CM.
LANEC_EV_FRONT_SRC = src/lanec_ev_front.c tests/lanec_ev_front_cases.c
LANEC_SUITE_SRC += $(LANEC_EV_FRONT_SRC)
LANEC_SUITE_HDR += include/lanec_ev_front.h
.PHONY: test-lanec-ev-front sanitize-lanec-ev-front c64-lanec-ev-front vice-lanec-ev-front
test: test-lanec-ev-front
sanitize: sanitize-lanec-ev-front
test-lanec-ev-front: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py ev-front
sanitize-lanec-ev-front: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py ev-front --native build/lanec-native-sanitize
c64-lanec-ev-front: build/lanec-ev-front.prg
build/lanec-ev-front.prg: $(LANEC_CORE) $(LANEC_EV_FRONT_SRC) $(LANEC_HDR) include/lanec_ev_front.h tests/lanec_ev_front_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_ev_front -DLANEC_FROZEN='"lanec_ev_front_c64.h"' -Wl,-Map,build/lanec-ev-front.map \
	  $(LANEC_CORE) $(LANEC_EV_FRONT_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-ev-front: build/lanec-ev-front.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-ev-front.prg
