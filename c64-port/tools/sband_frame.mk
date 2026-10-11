# SPDX-License-Identifier: MIT
CC ?= cc
SBFR_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBFR_INPUT = tests/sband_frame_cases.c src/agc_word.c
SBFR_CORE = $(SBFR_INPUT) src/sband_frame.c src/rtb_unit.c src/rtb_vector.c src/guidance_unit_phased.c src/guidance_length_phased.c src/guidance_norm.c src/lm_powered_interface.c src/lm_powered_trig.c src/agc_multiword.c src/guidance_product.c src/guidance_trig.c src/matrix_native.c src/sband_shift.c src/guidance_shift.c
build/sband-frame-native: tools/sband_frame_snapshot.c $(SBFR_CORE)
	$(CC) -O2 $(SBFR_FLAGS) $^ -o $@
build/sband-frame-reference: tools/sband_frame_reference.c $(SBFR_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBFR_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
all: build/sband-frame-native build/sband-frame-reference
test: all
	python3 tools/sband_frame_compare.py
sanitize:
	$(CC) -O1 -g $(SBFR_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_frame_snapshot.c $(SBFR_CORE) -o build/sband-frame-sanitize
	python3 tools/sband_frame_compare.py sanitize
.PHONY: all test sanitize
build/sband-frame-inputs: tools/sband_frame_inputs.c $(SBFR_INPUT)
	$(CC) -O2 $(SBFR_FLAGS) $^ -o $@
freeze: build/sband-frame-inputs
	python3 tools/freeze_sband_frame.py
.PHONY: freeze
