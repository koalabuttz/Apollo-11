# SPDX-License-Identifier: MIT
# Milestone lat-long (lane A2): Luminary099 and Comanche055 LATITUDE_LONGITUDE_SUBROUTINES compiled by
# tools/interp_compile.py into native routines of the control-transfer ABI.
LAT_LONG_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
LAT_LONG_SRC = src/lm_lat_long.c src/cm_lat_long.c tests/lat_long_target.c $(LAT_LONG_COMMON)
LAT_LONG_HDR = include/lm_lat_long.h include/cm_lat_long.h tests/interp_case.h tests/lat_long_vectors.h tests/lat_long_c64_vectors.h
LAT_LONG_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -fstatic-stack -mlto-zp=110
.PHONY: test-lat-long sanitize-lat-long c64-lat-long vice-lat-long lat-long-differential
all: build/test-lat-long
test: test-lat-long
sanitize: sanitize-lat-long
build/test-lat-long: $(LAT_LONG_SRC) tests/test_lat_long.c $(LAT_LONG_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LAT_LONG_SRC) tests/test_lat_long.c -o $@
test-lat-long: build/test-lat-long
	./build/test-lat-long
sanitize-lat-long: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LAT_LONG_SRC) tests/test_lat_long.c -o build/test-lat-long-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lat-long-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
lat-long-differential:
	python3 tools/interp_routine_differential.py lat-long --cases 256 --freeze
	git diff --exit-code -- tests/lat_long_vectors.h tests/lat_long_c64_vectors.h tools/mk/lat-long.groups
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; each fits below the $c000 mailbox).
include tools/mk/lat-long.groups
LAT_LONG_C64_PRGS = $(foreach g,$(LAT_LONG_GROUPS),build/lat-long-$(g).prg)
LAT_LONG_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/lat_long_target.c
LAT_LONG_C64_lm = src/lm_lat_long.c src/lm_interp_orders.c src/lm_interp_steps.c
LAT_LONG_C64_cm = src/cm_lat_long.c src/cm_interp_orders.c src/cm_interp_steps.c
c64-lat-long: $(LAT_LONG_C64_PRGS)
build/lat-long-%.prg: $(LAT_LONG_SRC) platform/c64/lat_long_diagnostic.c tools/mk/lat-long.groups $(LAT_LONG_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LAT_LONG_MOS_FLAGS) $(LAT_LONG_GROUP_FLAGS_$*) -Wl,-Map,build/lat-long-$*.map $(LAT_LONG_C64_$(LAT_LONG_GROUP_MISSION_$*)) $(LAT_LONG_C64_COMMON) platform/c64/lat_long_diagnostic.c -o $@
vice-lat-long: $(LAT_LONG_C64_PRGS)
	for p in $(LAT_LONG_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
