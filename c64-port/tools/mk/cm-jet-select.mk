# SPDX-License-Identifier: MIT
# Milestone cm-jet-select (lane G): Comanche055 JET_SELECTION_LOGIC compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_JET_SELECT_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_JET_SELECT_SRC = src/cm_jet_select.c tests/cm_jet_select_target.c $(CM_JET_SELECT_COMMON)
CM_JET_SELECT_HDR = include/cm_jet_select.h tests/interp_case.h tests/cm_jet_select_vectors.h tests/cm_jet_select_c64_vectors.h
CM_JET_SELECT_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS -DAGC_CHANNELS_HW=irc_cm_hw
.PHONY: test-cm-jet-select sanitize-cm-jet-select c64-cm-jet-select vice-cm-jet-select cm-jet-select-differential
all: build/test-cm-jet-select
test: test-cm-jet-select
sanitize: sanitize-cm-jet-select
build/test-cm-jet-select: $(CM_JET_SELECT_SRC) tests/test_cm_jet_select.c $(CM_JET_SELECT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_JET_SELECT_SRC) tests/test_cm_jet_select.c -o $@
test-cm-jet-select: build/test-cm-jet-select
	./build/test-cm-jet-select
# The hot table (spec hot_points, the flight store's docked RCS DAP packs)
# calls the full table's functions for its keys.
.PHONY: test-cm-jet-select-hot
all: build/test-cm-jet-select-hot
test: test-cm-jet-select-hot
build/test-cm-jet-select-hot: $(CM_JET_SELECT_SRC) tests/test_cm_jet_select_hot.c $(CM_JET_SELECT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_JET_SELECT_SRC) tests/test_cm_jet_select_hot.c -o $@
test-cm-jet-select-hot: build/test-cm-jet-select-hot
	./build/test-cm-jet-select-hot
sanitize-cm-jet-select: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_JET_SELECT_SRC) tests/test_cm_jet_select.c -o build/test-cm-jet-select-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-jet-select-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-jet-select-differential:
	python3 tools/interp_routine_differential.py cm-jet-select --cases 256 --freeze
	git diff --exit-code -- tests/cm_jet_select_vectors.h tests/cm_jet_select_c64_vectors.h tools/mk/cm-jet-select.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-jet-select.groups
CM_JET_SELECT_C64_PRGS = $(foreach g,$(CM_JET_SELECT_GROUPS),build/cm-jet-select-$(g).prg)
CM_JET_SELECT_C64_SRC = src/cm_jet_select.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_jet_select_target.c
c64-cm-jet-select: $(CM_JET_SELECT_C64_PRGS)
build/cm-jet-select-%.prg: $(CM_JET_SELECT_SRC) platform/c64/cm_jet_select_diagnostic.c tools/mk/cm-jet-select.groups $(CM_JET_SELECT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_JET_SELECT_MOS_FLAGS) $(CM_JET_SELECT_GROUP_FLAGS_$*) -Wl,-Map,build/cm-jet-select-$*.map $(CM_JET_SELECT_C64_SRC) platform/c64/cm_jet_select_diagnostic.c -o $@
vice-cm-jet-select: $(CM_JET_SELECT_C64_PRGS)
	for p in $(CM_JET_SELECT_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
