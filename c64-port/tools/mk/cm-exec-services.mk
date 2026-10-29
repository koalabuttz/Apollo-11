# SPDX-License-Identifier: MIT
# Milestone cm-exec-services (lane A): Comanche055 NOVAC, FINDVAC, WAITLIST,
# TWIDDLE, the LONGCALL chain, FIXDELAY, VARDELAY, TASKOVER and the T3RUPT
# vector as native routines of the control-transfer ABI, run from every
# genuine call site.
CM_EXS_SRC = src/cm_exec_routines.c src/cm_admission_zero.c src/cm_waitlist_extended.c src/cm_waitlist_internal.c src/cm_task_transfer.c src/cm_native_image.c src/cm_executive.c src/cm_waitlist.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/cm_exec_services_cases.c
CM_EXS_HDR = include/cm_exec_routines.h include/agc_transfer.h src/cm_exec_sites.inc tests/cm_exec_services_cases.h
CM_EXS_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-cm-exec-services sanitize-cm-exec-services c64-cm-exec-services vice-cm-exec-services cm-exec-services-differential
all: build/test-cm-exec-services
test: test-cm-exec-services
sanitize: sanitize-cm-exec-services
build/test-cm-exec-services: $(CM_EXS_SRC) tests/test_cm_exec_services.c tests/cm_exec_services_vectors.h $(CM_EXS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_EXS_FLAGS) $(CM_EXS_SRC) tests/test_cm_exec_services.c -o $@
test-cm-exec-services: build/test-cm-exec-services
	python3 tools/agc_sites_generate.py cm-exec --check
	./build/test-cm-exec-services
sanitize-cm-exec-services: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(CM_EXS_FLAGS) -fsanitize=address,undefined $(CM_EXS_SRC) tests/test_cm_exec_services.c -o build/test-cm-exec-services-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-exec-services-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Comanche055 listing build).
build/cm-exec-services-native: $(CM_EXS_SRC) platform/host/cm_exec_services_snapshot.c $(CM_EXS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_EXS_FLAGS) $(CM_EXS_SRC) platform/host/cm_exec_services_snapshot.c -o $@
cm-exec-services-differential: build/cm-exec-services-native
	python3 tools/cm_exec_services_freeze.py
	git diff --exit-code -- tests/cm_exec_services_vectors.h tests/cm_exec_services_c64_vectors.h
c64-cm-exec-services: build/cm-exec-services.prg
build/cm-exec-services.prg: $(CM_EXS_SRC) platform/c64/cm_exec_services_diagnostic.c tests/cm_exec_services_c64_vectors.h tests/cm_exec_services_c64_rows.h $(CM_EXS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(CM_EXS_FLAGS) -Wl,-Map,build/cm-exec-services.map $(CM_EXS_SRC) platform/c64/cm_exec_services_diagnostic.c -o $@
vice-cm-exec-services: build/cm-exec-services.prg
	python3 tools/vice_run.py build/cm-exec-services.prg --cycles 3000000000 --timeout 1200
