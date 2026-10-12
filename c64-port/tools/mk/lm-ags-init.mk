# SPDX-License-Identifier: MIT
# Milestone lm-ags-init (lane F): Luminary099 AGS_INITIALIZATION as native
# routines of the control-transfer ABI.
LM_AGS_SRC = src/lm_ags_init.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_ags_init_cases.c
LM_AGS_HDR = include/lm_ags_init.h include/lm_interp_orders.h include/lm_interp_steps.h include/agc_image_ops.h include/agc_transfer.h src/lm_ags_init_sites.inc tests/lm_ags_init_cases.h
LM_AGS_FLAGS = -Isrc -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-lm-ags-init sanitize-lm-ags-init c64-lm-ags-init vice-lm-ags-init lm-ags-init-differential
all: build/test-lm-ags-init
test: test-lm-ags-init
sanitize: sanitize-lm-ags-init
build/test-lm-ags-init: $(LM_AGS_SRC) tests/test_lm_ags_init.c tests/lm_ags_init_vectors.h $(LM_AGS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_AGS_FLAGS) -DAGS_COUNT_PATHS $(LM_AGS_SRC) tests/test_lm_ags_init.c -o $@
test-lm-ags-init: build/test-lm-ags-init
	./build/test-lm-ags-init
	python3 tools/lm_ags_init_source.py
sanitize-lm-ags-init: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(LM_AGS_FLAGS) -DAGS_COUNT_PATHS -fsanitize=address,undefined $(LM_AGS_SRC) tests/test_lm_ags_init.c -o build/test-lm-ags-init-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-ags-init-sanitize
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 listing build).
build/lm-ags-init-native: $(LM_AGS_SRC) platform/host/lm_ags_init_snapshot.c $(LM_AGS_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_AGS_FLAGS) $(LM_AGS_SRC) platform/host/lm_ags_init_snapshot.c -o $@
lm-ags-init-differential: build/lm-ags-init-native
	python3 tools/lm_ags_init_freeze.py
	git diff --exit-code -- tests/lm_ags_init_vectors.h tests/lm_ags_init_c64_vectors.h
c64-lm-ags-init: build/lm-ags-init.prg
build/lm-ags-init.prg: $(LM_AGS_SRC) platform/c64/lm_ags_init_diagnostic.c tests/lm_ags_init_c64_vectors.h $(LM_AGS_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Oz -DAPOLLO_FIXED_LM_ONLY -DP66_NATIVE_DISPATCH_ONLY -DAGS_UNDO_ADDRESS=0xf000u -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/lm-ags-init.map $(LM_AGS_SRC) platform/c64/lm_ags_init_diagnostic.c -o $@
vice-lm-ags-init: build/lm-ags-init.prg
	python3 tools/vice_run.py build/lm-ags-init.prg --cycles 3000000000 --timeout 1200
