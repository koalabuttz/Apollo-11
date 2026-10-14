# SPDX-License-Identifier: MIT
# Milestone lm-ground-track (lane A): the Luminary099 GROUND_TRACKING_DETERMINATION_PROGRAM routines compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
LM_GROUND_TRACK_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
LM_GROUND_TRACK_SRC = src/lm_ground_track.c tests/lm_ground_track_target.c $(LM_GROUND_TRACK_COMMON)
LM_GROUND_TRACK_HDR = include/lm_ground_track.h tests/interp_case.h tests/lm_ground_track_vectors.h tests/lm_ground_track_c64_vectors.h
LM_GROUND_TRACK_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS
.PHONY: test-lm-ground-track sanitize-lm-ground-track c64-lm-ground-track vice-lm-ground-track lm-ground-track-differential
all: build/test-lm-ground-track
test: test-lm-ground-track
sanitize: sanitize-lm-ground-track
build/test-lm-ground-track: $(LM_GROUND_TRACK_SRC) tests/test_lm_ground_track.c $(LM_GROUND_TRACK_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_GROUND_TRACK_SRC) tests/test_lm_ground_track.c -o $@
test-lm-ground-track: build/test-lm-ground-track
	./build/test-lm-ground-track
sanitize-lm-ground-track: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(LM_GROUND_TRACK_SRC) tests/test_lm_ground_track.c -o build/test-lm-ground-track-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-ground-track-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Luminary099 listing build).
lm-ground-track-differential:
	python3 tools/interp_routine_differential.py lm-ground-track --cases 512 --freeze
	git diff --exit-code -- tests/lm_ground_track_vectors.h tests/lm_ground_track_c64_vectors.h tools/mk/lm-ground-track.groups
# The differential again with the oracle tracing the words it executes
# and reads (tools/oracle_trace), then every instruction of the file checked
# executed (interpretive code: read by the interpreter).
.PHONY: lm-ground-track-closure
lm-ground-track-closure:
	rm -f build/lm-ground-track-executed.txt build/lm-ground-track-read.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/lm-ground-track-executed.txt AGC_READ_WORDS=$(CURDIR)/build/lm-ground-track-read.txt $(MAKE) --no-print-directory lm-ground-track-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py lm-ground-track
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/lm-ground-track.groups
LM_GROUND_TRACK_C64_PRGS = $(foreach g,$(LM_GROUND_TRACK_GROUPS),build/lm-ground-track-$(g).prg)
LM_GROUND_TRACK_C64_SRC = src/lm_ground_track.c src/lm_interp_orders.c src/lm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/lm_ground_track_target.c
c64-lm-ground-track: $(LM_GROUND_TRACK_C64_PRGS)
build/lm-ground-track-%.prg: $(LM_GROUND_TRACK_SRC) platform/c64/lm_ground_track_diagnostic.c tools/mk/lm-ground-track.groups $(LM_GROUND_TRACK_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(LM_GROUND_TRACK_MOS_FLAGS) $(LM_GROUND_TRACK_GROUP_FLAGS_$*) -Wl,-Map,build/lm-ground-track-$*.map $(LM_GROUND_TRACK_C64_SRC) platform/c64/lm_ground_track_diagnostic.c -o $@
vice-lm-ground-track: $(LM_GROUND_TRACK_C64_PRGS)
	for p in $(LM_GROUND_TRACK_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
