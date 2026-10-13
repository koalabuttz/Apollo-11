# SPDX-License-Identifier: MIT
# Milestone cm-tpi-search (lane E): Comanche055 TPI_SEARCH compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
CTPI_COMMON = src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CTPI_SRC = src/cm_tpi_search.c tests/cm_tpi_search_target.c $(CTPI_COMMON)
CTPI_HDR = include/cm_tpi_search.h tests/interp_case.h tests/cm_tpi_search_vectors.h tests/cm_tpi_search_c64_vectors.h
CTPI_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-cm-tpi-search sanitize-cm-tpi-search c64-cm-tpi-search vice-cm-tpi-search cm-tpi-search-differential
all: build/test-cm-tpi-search
test: test-cm-tpi-search
sanitize: sanitize-cm-tpi-search
build/test-cm-tpi-search: $(CTPI_SRC) tests/test_cm_tpi_search.c $(CTPI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CTPI_SRC) tests/test_cm_tpi_search.c -o $@
test-cm-tpi-search: build/test-cm-tpi-search
	./build/test-cm-tpi-search
sanitize-cm-tpi-search: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CTPI_SRC) tests/test_cm_tpi_search.c -o build/test-cm-tpi-search-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tpi-search-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
cm-tpi-search-differential:
	python3 tools/interp_routine_differential.py cm-tpi-search --cases 512 --freeze
	git diff --exit-code -- tests/cm_tpi_search_vectors.h tests/cm_tpi_search_c64_vectors.h tools/mk/cm-tpi-search.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/cm-tpi-search.groups
CTPI_C64_PRGS = $(foreach g,$(CM_TPI_SEARCH_GROUPS),build/cm-tpi-search-$(g).prg)
CTPI_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/interp_cut.c tests/cm_tpi_search_target.c
CTPI_C64_cm = src/cm_tpi_search.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-cm-tpi-search: $(CTPI_C64_PRGS)
build/cm-tpi-search-%.prg: $(CTPI_SRC) platform/c64/cm_tpi_search_diagnostic.c tools/mk/cm-tpi-search.groups $(CTPI_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CTPI_MOS_FLAGS) $(CM_TPI_SEARCH_GROUP_FLAGS_$*) -Wl,-Map,build/cm-tpi-search-$*.map $(CTPI_C64_$(CM_TPI_SEARCH_GROUP_MISSION_$*)) $(CTPI_C64_COMMON) platform/c64/cm_tpi_search_diagnostic.c -o $@
vice-cm-tpi-search: $(CTPI_C64_PRGS)
	for p in $(CTPI_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
