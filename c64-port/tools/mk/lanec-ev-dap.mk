# SPDX-License-Identifier: MIT
# Milestone lanec-ev-dap (lane C): EXTENDED_VERBS dap test verbs, LM.
LANEC_EV_DAP_SRC = src/lanec_ev_dap.c tests/lanec_ev_dap_cases.c
LANEC_SUITE_SRC += $(LANEC_EV_DAP_SRC)
LANEC_SUITE_HDR += include/lanec_ev_dap.h
.PHONY: test-lanec-ev-dap sanitize-lanec-ev-dap c64-lanec-ev-dap vice-lanec-ev-dap
test: test-lanec-ev-dap
sanitize: sanitize-lanec-ev-dap
test-lanec-ev-dap: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py ev-dap
sanitize-lanec-ev-dap: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py ev-dap --native build/lanec-native-sanitize
c64-lanec-ev-dap: build/lanec-ev-dap.prg
build/lanec-ev-dap.prg: $(LANEC_CORE) $(LANEC_EV_DAP_SRC) $(LANEC_HDR) include/lanec_ev_dap.h include/lanec_ev_internal.h tests/lanec_ev_dap_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_ev_dap -DLANEC_FROZEN='"lanec_ev_dap_c64.h"' -Wl,-Map,build/lanec-ev-dap.map \
	  $(LANEC_CORE) $(LANEC_EV_DAP_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-ev-dap: build/lanec-ev-dap.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-ev-dap.prg
