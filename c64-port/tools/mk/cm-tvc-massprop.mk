# SPDX-License-Identifier: MIT
# Milestone cm-tvc-massprop (lane G): Comanche055 TVCMASSPROP compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_TVC_MASSPROP_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_TVC_MASSPROP_SRC = src/cm_tvc_massprop.c tests/cm_tvc_massprop_target.c $(CM_TVC_MASSPROP_COMMON)
CM_TVC_MASSPROP_HDR = include/cm_tvc_massprop.h tests/interp_case.h tests/cm_tvc_massprop_vectors.h tests/cm_tvc_massprop_c64_vectors.h
CM_TVC_MASSPROP_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-cm-tvc-massprop sanitize-cm-tvc-massprop c64-cm-tvc-massprop vice-cm-tvc-massprop cm-tvc-massprop-differential
all: build/test-cm-tvc-massprop
test: test-cm-tvc-massprop
sanitize: sanitize-cm-tvc-massprop
build/test-cm-tvc-massprop: $(CM_TVC_MASSPROP_SRC) tests/test_cm_tvc_massprop.c $(CM_TVC_MASSPROP_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_TVC_MASSPROP_SRC) tests/test_cm_tvc_massprop.c -o $@
test-cm-tvc-massprop: build/test-cm-tvc-massprop
	./build/test-cm-tvc-massprop
sanitize-cm-tvc-massprop: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_TVC_MASSPROP_SRC) tests/test_cm_tvc_massprop.c -o build/test-cm-tvc-massprop-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-tvc-massprop-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-tvc-massprop-differential:
	python3 tools/interp_routine_differential.py cm-tvc-massprop --cases 256 --freeze
	git diff --exit-code -- tests/cm_tvc_massprop_vectors.h tests/cm_tvc_massprop_c64_vectors.h tools/mk/cm-tvc-massprop.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-tvc-massprop.groups
CM_TVC_MASSPROP_C64_PRGS = $(foreach g,$(CM_TVC_MASSPROP_GROUPS),build/cm-tvc-massprop-$(g).prg)
CM_TVC_MASSPROP_C64_SRC = src/cm_tvc_massprop.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_tvc_massprop_target.c
c64-cm-tvc-massprop: $(CM_TVC_MASSPROP_C64_PRGS)
build/cm-tvc-massprop-%.prg: $(CM_TVC_MASSPROP_SRC) platform/c64/cm_tvc_massprop_diagnostic.c tools/mk/cm-tvc-massprop.groups $(CM_TVC_MASSPROP_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_TVC_MASSPROP_MOS_FLAGS) $(CM_TVC_MASSPROP_GROUP_FLAGS_$*) -Wl,-Map,build/cm-tvc-massprop-$*.map $(CM_TVC_MASSPROP_C64_SRC) platform/c64/cm_tvc_massprop_diagnostic.c -o $@
vice-cm-tvc-massprop: $(CM_TVC_MASSPROP_C64_PRGS)
	for p in $(CM_TVC_MASSPROP_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
