# SPDX-License-Identifier: MIT
# Milestone lm-sps-backup (lane F): Luminary099 SPS_BACK-UP_RCS_CONTROL compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
SPB_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
SPB_SRC = src/lm_sps_backup.c tests/lm_sps_backup_target.c $(SPB_COMMON)
SPB_HDR = include/lm_sps_backup.h tests/interp_case.h tests/lm_sps_backup_vectors.h tests/lm_sps_backup_c64_vectors.h
SPB_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-lm-sps-backup sanitize-lm-sps-backup c64-lm-sps-backup vice-lm-sps-backup lm-sps-backup-differential
all: build/test-lm-sps-backup
test: test-lm-sps-backup
sanitize: sanitize-lm-sps-backup
build/test-lm-sps-backup: $(SPB_SRC) tests/test_lm_sps_backup.c $(SPB_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SPB_SRC) tests/test_lm_sps_backup.c -o $@
test-lm-sps-backup: build/test-lm-sps-backup
	./build/test-lm-sps-backup
sanitize-lm-sps-backup: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(SPB_SRC) tests/test_lm_sps_backup.c -o build/test-lm-sps-backup-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-sps-backup-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
lm-sps-backup-differential:
	python3 tools/interp_routine_differential.py lm-sps-backup --cases 512 --freeze
	git diff --exit-code -- tests/lm_sps_backup_vectors.h tests/lm_sps_backup_c64_vectors.h tools/mk/lm-sps-backup.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
-include tools/mk/lm-sps-backup.groups
SPB_C64_PRGS = $(foreach g,$(LM_SPS_BACKUP_GROUPS),build/lm-sps-backup-$(g).prg)
SPB_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/lm_sps_backup_target.c
SPB_C64_lm = src/lm_sps_backup.c src/lm_interp_orders.c src/lm_interp_steps.c
c64-lm-sps-backup: $(SPB_C64_PRGS)
build/lm-sps-backup-%.prg: $(SPB_SRC) platform/c64/lm_sps_backup_diagnostic.c tools/mk/lm-sps-backup.groups $(SPB_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(SPB_MOS_FLAGS) $(LM_SPS_BACKUP_GROUP_FLAGS_$*) -Wl,-Map,build/lm-sps-backup-$*.map $(SPB_C64_$(LM_SPS_BACKUP_GROUP_MISSION_$*)) $(SPB_C64_COMMON) platform/c64/lm_sps_backup_diagnostic.c -o $@
vice-lm-sps-backup: $(SPB_C64_PRGS)
	for p in $(SPB_C64_PRGS); do echo "PRG $$(basename $$p) sha256 $$(sha256sum $$p | cut -c1-64)"; python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
