# SPDX-License-Identifier: MIT
# Milestone lanec-ev-lmi (lane C): EXTENDED_VERBS interpretive words, LM.
LANEC_LMI_DEPS = src/lm_interp_steps.c src/lm_interp_orders.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_multiword.c
LANEC_EV_LMI_SRC = src/lanec_ev_lmi.c tests/lanec_ev_lmi_cases.c
LANEC_SUITE_SRC += $(LANEC_EV_LMI_SRC) $(LANEC_LMI_DEPS)
LANEC_SUITE_HDR += include/lanec_ev_lmi.h
.PHONY: test-lanec-ev-lmi sanitize-lanec-ev-lmi c64-lanec-ev-lmi vice-lanec-ev-lmi
test: test-lanec-ev-lmi
sanitize: sanitize-lanec-ev-lmi
test-lanec-ev-lmi: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/symbols.py
	python3 tools/lanec/diff.py ev-lmi
sanitize-lanec-ev-lmi: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py ev-lmi --native build/lanec-native-sanitize
c64-lanec-ev-lmi: build/lanec-ev-lmi.prg
build/lanec-ev-lmi.prg: $(LANEC_CORE) $(LANEC_EV_LMI_SRC) $(LANEC_LMI_DEPS) $(LANEC_HDR) include/lanec_ev_lmi.h tests/lanec_ev_lmi_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests \
	  -DLANEC_SUITE=lanec_suite_ev_lmi -DLANEC_FROZEN='"lanec_ev_lmi_c64.h"' -Wl,-Map,build/lanec-ev-lmi.map \
	  $(LANEC_CORE) $(LANEC_EV_LMI_SRC) $(LANEC_LMI_DEPS) tools/lanec/c64_diag.c -o $@
vice-lanec-ev-lmi: build/lanec-ev-lmi.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-ev-lmi.prg
