# SPDX-License-Identifier: MIT
# Milestone geometry (lane D1b): Luminary099 and Comanche055
# LEM_GEOMETRY and CSM_GEOMETRY compiled by tools/interp_compile.py into
# native routines of the control-transfer ABI.
GEO_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c
GEO_SRC = src/lm_geometry.c src/cm_geometry.c tests/geometry_target.c $(GEO_COMMON)
GEO_HDR = include/lm_geometry.h include/cm_geometry.h tests/interp_case.h tests/geometry_vectors.h tests/geometry_c64_vectors.h
GEO_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror
.PHONY: test-geometry sanitize-geometry c64-geometry vice-geometry geometry-differential
all: build/test-geometry
test: test-geometry
sanitize: sanitize-geometry
build/test-geometry: $(GEO_SRC) tests/test_geometry.c $(GEO_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GEO_SRC) tests/test_geometry.c -o $@
test-geometry: build/test-geometry
	./build/test-geometry
sanitize-geometry: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(GEO_SRC) tests/test_geometry.c -o build/test-geometry-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-geometry-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
geometry-differential:
	python3 tools/interp_routine_differential.py geometry --cases 512 --freeze
	git diff --exit-code -- tests/geometry_vectors.h tests/geometry_c64_vectors.h tools/mk/geometry.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/geometry.groups
GEO_C64_PRGS = $(foreach g,$(GEOMETRY_GROUPS),build/geometry-$(g).prg)
GEO_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/rtb_angles.c tests/interp_case.c tests/geometry_target.c
GEO_C64_lm = src/lm_geometry.c src/lm_interp_orders.c src/lm_interp_steps.c
GEO_C64_cm = src/cm_geometry.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-geometry: $(GEO_C64_PRGS)
build/geometry-%.prg: $(GEO_SRC) platform/c64/geometry_diagnostic.c tools/mk/geometry.groups $(GEO_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(GEO_MOS_FLAGS) $(GEOMETRY_GROUP_FLAGS_$*) -Wl,-Map,build/geometry-$*.map $(GEO_C64_$(GEOMETRY_GROUP_MISSION_$*)) $(GEO_C64_COMMON) platform/c64/geometry_diagnostic.c -o $@
vice-geometry: $(GEO_C64_PRGS)
	for p in $(GEO_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
