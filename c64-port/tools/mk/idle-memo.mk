# SPDX-License-Identifier: MIT
# The resident's idle memo (Resident kernel lane, docs/RESIDENT-FLIGHT.md,
# Idle memo): tests/test_idle_memo.c runs the idle loop's two steps on random
# images and checks they read only the memo's key and write only its result.
IDLE_MEMO_LM_SRC = $(sort $(RK_LM_SRC) src/lm_self_check_idle.c src/lm_interbank_dap.c src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c src/agc_hardware_divide.c)
IDLE_MEMO_CM_SRC = $(sort $(RK_CM_SRC) src/cm_self_check_idle.c src/cm_interbank_postjump.c src/cm_interbank_routines.c src/cm_interbank_idle.c src/cm_interbank_complete.c src/agc_hardware_divide.c)
.PHONY: test-idle-memo
all: build/test-idle-memo-lm build/test-idle-memo-cm
test: test-idle-memo
build/test-idle-memo-lm: $(IDLE_MEMO_LM_SRC) tests/test_idle_memo.c $(RK_HDR) include/self_check.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RK_LM_FLAGS) -DRK_SEAMS=1 $(IDLE_MEMO_LM_SRC) tests/test_idle_memo.c -o $@
build/test-idle-memo-cm: $(IDLE_MEMO_CM_SRC) tests/test_idle_memo.c $(RK_HDR) include/self_check.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RK_CM_FLAGS) -DRK_SEAMS=1 $(IDLE_MEMO_CM_SRC) tests/test_idle_memo.c -o $@
test-idle-memo: build/test-idle-memo-lm build/test-idle-memo-cm
	./build/test-idle-memo-lm
	./build/test-idle-memo-cm
