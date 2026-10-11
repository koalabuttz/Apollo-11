# SPDX-License-Identifier: MIT
CC ?= cc
SBCA_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBCA_INPUT = tests/sband_lm_yaw_cases.c src/agc_word.c
SBCA_CORE = $(SBCA_INPUT) src/sband_lm_yaw.c src/sband_trig.c src/guidance_trig.c src/guidance_sum.c src/guidance_divide.c src/sband_vector_linear.c src/sband_vector.c src/sband_shift.c src/sband_length.c src/guidance_length_phased.c src/interpreted_sign.c src/sband_dot.c src/sband_scalar.c src/inverse_trig.c src/guidance_shift.c src/guidance_product.c src/guidance_norm.c src/agc_multiword.c
build/sband-lm-yaw-native: tools/sband_lm_yaw_snapshot.c $(SBCA_CORE)
	$(CC) -O2 $(SBCA_FLAGS) $^ -o $@
build/sband-lm-yaw-reference: tools/sband_lm_yaw_reference.c $(SBCA_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBCA_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
all: build/sband-lm-yaw-native build/sband-lm-yaw-reference
test: all
	python3 tools/sband_lm_yaw_compare.py
sanitize:
	$(CC) -O1 -g $(SBCA_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_lm_yaw_snapshot.c $(SBCA_CORE) -o build/sband-lm-yaw-sanitize
	python3 tools/sband_lm_yaw_compare.py sanitize
.PHONY: all test sanitize
