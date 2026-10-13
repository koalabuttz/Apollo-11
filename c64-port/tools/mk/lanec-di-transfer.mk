# SPDX-License-Identifier: MIT
# Milestone lanec-di-transfer (lane C): DISPLAY_INTERFACE_ROUTINES, LM and CM,
# as routines of the control-transfer ABI, with round trips from every genuine
# BANKCALL/POSTJUMP caller through INTER-BANK_COMMUNICATION.
LANEC_DI_ABI = src/lanec_di_routines.c src/agc_transfer.c src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c \
  src/cm_interbank_routines.c src/cm_interbank_idle.c src/cm_interbank_complete.c
# Luminary099 Executive job services (lane A, as in lm-exec-jobs), so LM round
# trips continue through PRIOCHNG, JOBSLEEP, NOVAC... back into the file.
LANEC_LM_EXEC = src/lm_exec_jobs.c src/lm_exec_routines.c src/lm_admission_image.c src/lm_basic_sleep.c \
  src/lm_basic_wake.c src/lm_basic_priority.c src/lm_basic_finish.c src/lm_basic_change.c \
  src/lm_interpreter_change.c src/lm_waitlist_image.c src/waitlist.c src/executive.c tests/lanec_p66_dispatch.c
LANEC_DI_TRANSFER_SRC = $(LANEC_DI_ABI) $(LANEC_LM_EXEC) tests/lanec_di_transfer_cases.c
LANEC_DI_SRC = src/lanec_di_dsp.c tests/lanec_di_dsp_cases.c src/lanec_di_mark.c tests/lanec_di_mark_cases.c \
  src/lanec_di_play.c tests/lanec_di_play_cases.c src/lanec_di_sleep.c tests/lanec_di_sleep_cases.c \
  src/lanec_di_idle.c tests/lanec_di_idle_cases.c src/lanec_di_nvdsp.c tests/lanec_di_nvdsp_cases.c
LANEC_SUITE_SRC += $(LANEC_DI_TRANSFER_SRC)
LANEC_SUITE_HDR += include/lanec_di_routines.h src/lanec_di_routines.inc tests/lanec_di_transfer_cases.inc \
  src/lm_interbank_sites.inc src/cm_interbank_sites.inc src/lm_exec_sites.inc include/lm_exec_routines.h
.PHONY: test-lanec-di-transfer sanitize-lanec-di-transfer c64-lanec-di-transfer vice-lanec-di-transfer \
  c64-lanec-di-transfer-cm vice-lanec-di-transfer-cm
test: test-lanec-di-transfer
sanitize: sanitize-lanec-di-transfer
test-lanec-di-transfer: build/lanec-native build/lanec-oracle tools/lanec/diff.py tools/lanec/routines.py
	python3 tools/lanec/routines.py di --check
	python3 tools/lanec/diff.py di-transfer
	python3 tools/lanec/diff.py di-transfer-c64 --refusals
sanitize-lanec-di-transfer: build/lanec-native-sanitize build/lanec-oracle tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py di-transfer --native build/lanec-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py di-transfer-c64 --refusals --native build/lanec-native-sanitize
# The C64 runs di-transfer-c64's frozen rows (the whole round trip does not fit
# one program): INTER-BANK, display-start and MAKEPLAY routines only, one
# program per mission (LANEC_DI_C64_CM), without inlining and with the static
# stack, to fit below the mailbox page ($c000).
LANEC_DI_TRANSFER_PRG_SRC = $(LANEC_CORE) $(LANEC_DI_ABI) tests/lanec_di_transfer_cases.c src/lanec_di_dsp.c \
  tests/lanec_di_dsp_cases.c src/lanec_di_play.c tests/lanec_di_play_cases.c src/lanec_di_mark.c src/lanec_di_sleep.c \
  src/lanec_di_idle.c src/lanec_di_nvdsp.c tools/lanec/c64_diag.c
LANEC_DI_TRANSFER_PRG_CC = $(MOS_CC) -std=c99 -Os -fno-inline -mlto-zp=0 -Wall -Wextra -Werror \
  -Iinclude -Itests -DLANEC_DI_TRANSFER_C64 -DLANEC_SUITE=lanec_suite_di_transfer_c64 \
  -DLANEC_FROZEN='"lanec_di_transfer_c64_c64.h"' -DLANEC_C64_SKIP=lanec_di_c64_skip
c64-lanec-di-transfer: build/lanec-di-transfer.prg build/lanec-di-transfer-cm.prg
c64-lanec-di-transfer-cm: build/lanec-di-transfer-cm.prg
build/lanec-di-transfer.prg: $(LANEC_DI_TRANSFER_PRG_SRC) $(LANEC_HDR) include/lanec_di_routines.h tests/lanec_di_transfer_c64_c64.h | build
	$(LANEC_DI_TRANSFER_PRG_CC) -Wl,-Map,build/lanec-di-transfer.map $(LANEC_DI_TRANSFER_PRG_SRC) -o $@
build/lanec-di-transfer-cm.prg: $(LANEC_DI_TRANSFER_PRG_SRC) $(LANEC_HDR) include/lanec_di_routines.h tests/lanec_di_transfer_c64_c64.h | build
	$(LANEC_DI_TRANSFER_PRG_CC) -DLANEC_DI_C64_CM -Wl,-Map,build/lanec-di-transfer-cm.map $(LANEC_DI_TRANSFER_PRG_SRC) -o $@
vice-lanec-di-transfer-cm: build/lanec-di-transfer-cm.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-di-transfer-cm.prg
vice-lanec-di-transfer: vice-lanec-di-transfer-cm build/lanec-di-transfer.prg
	python3 tools/vice_run.py --cycles 900000000 build/lanec-di-transfer.prg
# di-refuse (milestone lanec-di-transfer): out-of-domain entry states refused unchanged.
LANEC_SUITE_SRC += tests/lanec_di_refuse_cases.c
test-lanec-di-transfer: test-lanec-di-refuse
sanitize-lanec-di-transfer: sanitize-lanec-di-refuse
.PHONY: test-lanec-di-refuse sanitize-lanec-di-refuse
test-lanec-di-refuse: build/lanec-native tools/lanec/diff.py
	python3 tools/lanec/diff.py di-refuse --refusals-only
sanitize-lanec-di-refuse: build/lanec-native-sanitize tools/lanec/diff.py
	ASAN_OPTIONS=detect_leaks=0 python3 tools/lanec/diff.py di-refuse --refusals-only --native build/lanec-native-sanitize
