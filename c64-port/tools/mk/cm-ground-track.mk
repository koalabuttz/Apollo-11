# SPDX-License-Identifier: MIT
# Milestone cm-ground-track (lane A): the Comanche055 GROUND_TRACKING_DETERMINATION_PROGRAM (P21) routines compiled by
# tools/interp_compile.py and tools/basic_compile.py into native routines of
# the control-transfer ABI.
CM_GROUND_TRACK_COMMON = src/lm_interp_orders.c src/lm_interp_steps.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c
CM_GROUND_TRACK_SRC = src/cm_ground_track.c tests/cm_ground_track_target.c $(CM_GROUND_TRACK_COMMON)
CM_GROUND_TRACK_HDR = include/cm_ground_track.h tests/interp_case.h tests/cm_ground_track_vectors.h tests/cm_ground_track_c64_vectors.h
CM_GROUND_TRACK_MOS_FLAGS = -std=c99 -Oz -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -fno-inline -DIRC_CHANNELS -DAGC_CHANNELS_HW=irc_cm_hw
.PHONY: test-cm-ground-track sanitize-cm-ground-track c64-cm-ground-track vice-cm-ground-track cm-ground-track-differential
all: build/test-cm-ground-track
test: test-cm-ground-track
sanitize: sanitize-cm-ground-track
build/test-cm-ground-track: $(CM_GROUND_TRACK_SRC) tests/test_cm_ground_track.c $(CM_GROUND_TRACK_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CM_GROUND_TRACK_SRC) tests/test_cm_ground_track.c -o $@
test-cm-ground-track: build/test-cm-ground-track
	./build/test-cm-ground-track
sanitize-cm-ground-track: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_GROUND_TRACK_SRC) tests/test_cm_ground_track.c -o build/test-cm-ground-track-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-ground-track-sanitize
# Host-only oracle comparison: the original rope in yaAGC against the native
# routines (needs deps/virtualagc and the Comanche055 listing build).
cm-ground-track-differential:
	python3 tools/interp_routine_differential.py cm-ground-track --cases 512 --freeze
	git diff --exit-code -- tests/cm_ground_track_vectors.h tests/cm_ground_track_c64_vectors.h tools/mk/cm-ground-track.groups
# The differential again with the oracle tracing the words it executes
# and reads (tools/oracle_trace), then every instruction of the file checked
# executed (interpretive code: read by the interpreter).
.PHONY: cm-ground-track-closure
cm-ground-track-closure:
	rm -f build/cm-ground-track-executed.txt build/cm-ground-track-read.txt build/interp-routine-reference build/interp-routine-reference-flight-device
	PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/cm-ground-track-executed.txt AGC_READ_WORDS=$(CURDIR)/build/cm-ground-track-read.txt $(MAKE) --no-print-directory cm-ground-track-differential
	rm -f build/interp-routine-reference build/interp-routine-reference-flight-device
	python3 tools/interp_executed_closure.py cm-ground-track
# The C64 diagnostic is one program per group of routines (generated with
# the vectors; the image sits under the KERNAL ROM, the code below $c000).
include tools/mk/cm-ground-track.groups
CM_GROUND_TRACK_C64_PRGS = $(foreach g,$(CM_GROUND_TRACK_GROUPS),build/cm-ground-track-$(g).prg)
CM_GROUND_TRACK_C64_SRC = src/cm_ground_track.c src/cm_interp_orders.c src/cm_interp_steps.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/interp_case.c tests/cm_ground_track_target.c
c64-cm-ground-track: $(CM_GROUND_TRACK_C64_PRGS)
build/cm-ground-track-%.prg: $(CM_GROUND_TRACK_SRC) platform/c64/cm_ground_track_diagnostic.c tools/mk/cm-ground-track.groups $(CM_GROUND_TRACK_HDR) | build
	$(MOS_CC) $(CPPFLAGS) $(CM_GROUND_TRACK_MOS_FLAGS) $(CM_GROUND_TRACK_GROUP_FLAGS_$*) -Wl,-Map,build/cm-ground-track-$*.map $(CM_GROUND_TRACK_C64_SRC) platform/c64/cm_ground_track_diagnostic.c -o $@
vice-cm-ground-track: $(CM_GROUND_TRACK_C64_PRGS)
	for p in $(CM_GROUND_TRACK_C64_PRGS); do python3 tools/vice_run.py $$p --cycles 3000000000 --timeout 1200 || exit 1; done
