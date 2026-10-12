# SPDX-License-Identifier: MIT
# Milestone gimbal-lock (lane D1): Luminary099 GIMBAL_LOCK_AVOIDANCE
# and Comanche055 GIMBAL_LOCK_AVOIDANCE compiled by tools/interp_compile.py into
# native routines of the control-transfer ABI.
GLA_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
GLA_SRC = src/lm_gimbal_lock.c src/cm_gimbal_lock.c tests/gimbal_lock_target.c $(GLA_COMMON)
GLA_HDR = include/lm_gimbal_lock.h include/cm_gimbal_lock.h tests/interp_case.h tests/gimbal_lock_vectors.h tests/gimbal_lock_c64_vectors.h
GLA_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-gimbal-lock sanitize-gimbal-lock c64-gimbal-lock vice-gimbal-lock gimbal-lock-differential
all: build/test-gimbal-lock
test: test-gimbal-lock
sanitize: sanitize-gimbal-lock
build/test-gimbal-lock: $(GLA_SRC) tests/test_gimbal_lock.c $(GLA_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GLA_SRC) tests/test_gimbal_lock.c -o $@
test-gimbal-lock: build/test-gimbal-lock
	./build/test-gimbal-lock
sanitize-gimbal-lock: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(GLA_SRC) tests/test_gimbal_lock.c -o build/test-gimbal-lock-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-gimbal-lock-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
gimbal-lock-differential:
	python3 tools/interp_routine_differential.py gimbal-lock --cases 512 --freeze
	git diff --exit-code -- tests/gimbal_lock_vectors.h tests/gimbal_lock_c64_vectors.h tools/mk/gimbal-lock.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/gimbal-lock.groups
GLA_C64_PRGS = $(foreach g,$(GIMBAL_LOCK_GROUPS),build/gimbal-lock-$(g).prg)
GLA_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/gimbal_lock_target.c
GLA_C64_lm = src/lm_gimbal_lock.c src/lm_interp_orders.c src/lm_interp_steps.c
GLA_C64_cm = src/cm_gimbal_lock.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-gimbal-lock: $(GLA_C64_PRGS)
build/gimbal-lock-%.prg: $(GLA_SRC) platform/c64/gimbal_lock_diagnostic.c tools/mk/gimbal-lock.groups $(GLA_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(GLA_MOS_FLAGS) $(GIMBAL_LOCK_GROUP_FLAGS_$*) -Wl,-Map,build/gimbal-lock-$*.map $(GLA_C64_$(GIMBAL_LOCK_GROUP_MISSION_$*)) $(GLA_C64_COMMON) platform/c64/gimbal_lock_diagnostic.c -o $@
vice-gimbal-lock: $(GLA_C64_PRGS)
	for p in $(GLA_C64_PRGS); do sha256sum $$p; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
