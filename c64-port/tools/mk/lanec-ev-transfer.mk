# SPDX-License-Identifier: MIT
# Milestone lanec-ev-transfer (lane C): EXTENDED_VERBS, LM and CM, as routines
# of the control-transfer ABI, with round trips from every genuine
# BANKCALL/POSTJUMP caller (Pinball's POSTJUMP GOEXTVB among them) through
# INTER-BANK_COMMUNICATION, the display interface, the LM Executive and the
# LM interpreter's INTPRET.
LANEC_EV_ABI = src/lanec_ev_routines.c src/standard_lead_routines.c src/interp_entry.c src/standard_lead.c src/standard_lead_interp.c \
  src/standard_lead_wake.c src/cm_basic_wake.c src/cm_native_image.c src/cm_dispatch.c src/cm_executive.c
LANEC_EV_TRANSFER_SRC = $(LANEC_EV_ABI) tests/lanec_ev_transfer_cases.c
LANEC_SUITE_SRC += $(LANEC_EV_TRANSFER_SRC)
LANEC_SUITE_HDR += include/lanec_ev_routines.h src/lanec_ev_routines.inc tests/lanec_ev_transfer_cases.inc
.PHONY: test-lanec-ev-transfer sanitize-lanec-ev-transfer
test: test-lanec-ev-transfer
sanitize: sanitize-lanec-ev-transfer
test-lanec-ev-transfer: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/routines.py
	python3 tools/lanec/routines.py ev --check
	python3 tools/lanec/diff.py ev-transfer
	python3 tools/lanec/diff.py ev-transfer-c64
sanitize-lanec-ev-transfer: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py ev-transfer --native build/lanec-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py ev-transfer-c64 --native build/lanec-native-sanitize
# The C64 runs ev-transfer-c64 (the whole round trip does not fit one program):
# INTER-BANK and the front routines only; the other modules are linked for
# the routine table and dropped by link-time optimization.
LANEC_EV_C64_SRC = src/lanec_ev_routines.c src/agc_transfer.c src/lm_interbank_routines.c src/lm_interbank.c \
  src/lanec_ev_front.c tests/lanec_ev_front_cases.c src/lanec_ev_short.c src/lanec_ev_align.c src/lanec_ev_radar.c \
  src/lanec_ev_dap.c src/lanec_ev_time.c src/lanec_ev_track.c src/lanec_ev_lmi.c $(LANEC_LMI_DEPS) \
  tests/lanec_ev_transfer_cases.c
.PHONY: c64-lanec-ev-transfer vice-lanec-ev-transfer
c64-lanec-ev-transfer: build/lanec-ev-transfer.prg
build/lanec-ev-transfer.prg: $(LANEC_CORE) $(LANEC_EV_C64_SRC) $(LANEC_HDR) include/lanec_ev_routines.h tests/lanec_ev_transfer_c64_c64.h tools/lanec/c64_diag.c | build
	$(MOS_CC) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Iinclude -Itests -DLANEC_EV_TRANSFER_C64 \
	  -DLANEC_SUITE=lanec_suite_ev_transfer_c64 -DLANEC_FROZEN='"lanec_ev_transfer_c64_c64.h"' -Wl,-Map,build/lanec-ev-transfer.map \
	  $(LANEC_CORE) $(LANEC_EV_C64_SRC) tools/lanec/c64_diag.c -o $@
vice-lanec-ev-transfer: build/lanec-ev-transfer.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-ev-transfer.prg
# ev-refuse (milestone lanec-ev-transfer): out-of-domain entry states refused unchanged.
LANEC_SUITE_SRC += tests/lanec_ev_refuse_cases.c
test-lanec-ev-transfer: test-lanec-ev-refuse
sanitize-lanec-ev-transfer: sanitize-lanec-ev-refuse
.PHONY: test-lanec-ev-refuse sanitize-lanec-ev-refuse
test-lanec-ev-refuse: build/lanec-native tools/lanec/diff.py
	python3 tools/lanec/diff.py ev-refuse --refusals-only
sanitize-lanec-ev-refuse: build/lanec-native-sanitize tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py ev-refuse --refusals-only --native build/lanec-native-sanitize
