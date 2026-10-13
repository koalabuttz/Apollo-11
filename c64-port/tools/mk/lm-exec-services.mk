# SPDX-License-Identifier: MIT
# Milestone lm-exec-services (lane A): Luminary099 NOVAC, FINDVAC, WAITLIST,
# TWIDDLE and the LONGCALL chain as native routines of the control-transfer
# ABI, composed with RESTARTS and with every genuine call site.
LM_EXS_SRC = src/lm_exec_routines.c src/lm_admission_image.c src/lm_waitlist_image.c src/waitlist.c src/executive.c src/p66_native.c src/lm_restart_select.c src/lm_restart_routines.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_restart_select_cases.c tests/lm_restart_routines_cases.c tests/lm_exec_services_cases.c
LM_EXS_HDR = include/lm_exec_routines.h include/agc_transfer.h src/lm_exec_sites.inc src/lm_restart_table_data.inc tests/lm_exec_services_cases.h tests/lm_restart_routines_cases.h
LM_EXS_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-exec-services sanitize-lm-exec-services c64-lm-exec-services vice-lm-exec-services lm-exec-services-differential
all: build/test-lm-exec-services
test: test-lm-exec-services
sanitize: sanitize-lm-exec-services
build/test-lm-exec-services: $(LM_EXS_SRC) tests/test_lm_exec_services.c tests/lm_exec_services_vectors.h $(LM_EXS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_EXS_FLAGS) $(LM_EXS_SRC) tests/test_lm_exec_services.c -o $@
test-lm-exec-services: build/test-lm-exec-services
	python3 tools/agc_sites_generate.py lm-exec --check
	./build/test-lm-exec-services
sanitize-lm-exec-services: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_EXS_FLAGS) -fsanitize=address,undefined $(LM_EXS_SRC) tests/test_lm_exec_services.c -o build/test-lm-exec-services-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-exec-services-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-exec-services-native: $(LM_EXS_SRC) platform/host/lm_exec_services_snapshot.c $(LM_EXS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_EXS_FLAGS) $(LM_EXS_SRC) platform/host/lm_exec_services_snapshot.c -o $@
lm-exec-services-differential: build/lm-exec-services-native
	python3 tools/lm_exec_services_freeze.py
	git diff --exit-code -- tests/lm_exec_services_vectors.h tests/lm_exec_services_c64_vectors.h
c64-lm-exec-services: build/lm-exec-services.prg
build/lm-exec-services.prg: $(LM_EXS_SRC) platform/c64/lm_exec_services_diagnostic.c tests/lm_exec_services_c64_vectors.h tests/lm_exec_services_c64_rows.h $(LM_EXS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -DP66_NATIVE_DISPATCH_ONLY -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-exec-services.map $(LM_EXS_SRC) platform/c64/lm_exec_services_diagnostic.c -o $@
vice-lm-exec-services: build/lm-exec-services.prg
	python3 tools/vice_run.py build/lm-exec-services.prg --cycles 3000000000 --timeout 1200
