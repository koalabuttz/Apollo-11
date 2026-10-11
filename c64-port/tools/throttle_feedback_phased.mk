# SPDX-License-Identifier: MIT
# Standalone milestone build; the parent owns integration into the main Makefile.
TFP_FLAGS = -DTHROTTLE_FEEDBACK_INPUT_ONLY -std=c99 -Wall -Wextra -Werror -Iinclude -Itests -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
TFP_CORE = src/guidance_throttle_phased.c src/guidance_product.c src/guidance_vector.c src/guidance_unit.c src/guidance_sum.c src/guidance_shift.c src/guidance_gravity.c src/guidance_norm.c src/guidance_dp_divide.c src/agc_word.c src/agc_multiword.c tests/scenarios.c tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/throttle_feedback_cases.c tests/throttle_feedback_phased_cases.c
TFP_TEST = tests/test_throttle_feedback_phased.c tests/frozen_words.c $(TFP_CORE)
.PHONY: throttle_feedback_phased-host throttle_feedback_phased-sanitize throttle_feedback_phased-c64
throttle_feedback_phased-host:
	gcc -O2 -Wconversion -pedantic $(TFP_FLAGS) $(TFP_TEST) -o build/test-throttle_feedback_phased
	build/test-throttle_feedback_phased
throttle_feedback_phased-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(TFP_FLAGS) $(TFP_TEST) -o build/test-throttle_feedback_phased-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-throttle_feedback_phased-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(TFP_FLAGS) tools/throttle_feedback_phased_snapshot.c $(TFP_CORE) -o build/throttle_feedback_phased-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/throttle_feedback_phased-native-sanitize > build/throttle_feedback_phased-sanitize.snapshots
	cmp build/throttle_feedback_phased-sanitize.snapshots build/throttle_feedback_phased-reference.snapshots
throttle_feedback_phased-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DFEEDBACK_PHASE_CHECK_ONLY -DFEEDBACK_PHASE_SELECTED=$(if $(PHASE),$(PHASE),0) $(TFP_FLAGS) tools/throttle_feedback_phased_diagnostic.c $(TFP_TEST) -Wl,-Map,build/feedback-phase$(if $(PHASE),$(PHASE),0).map -o build/feedback-phase$(if $(PHASE),$(PHASE),0).prg

all: build/throttle_feedback_phased-native build/test-throttle_feedback_phased
build/throttle_feedback_phased-native: $(TFP_CORE) tools/throttle_feedback_phased_snapshot.c tests/throttle_feedback_cases.h include/guidance_throttle_phased.h | build
	$(CC) -O2 $(TFP_FLAGS) tools/throttle_feedback_phased_snapshot.c $(TFP_CORE) -o $@
build/test-throttle_feedback_phased: $(TFP_TEST) tests/throttle_feedback_phased_vectors.h tests/throttle_feedback_cases.h include/guidance_throttle_phased.h | build
	$(CC) -O2 -Wconversion -pedantic $(TFP_FLAGS) $(TFP_TEST) -o $@
test: throttle_feedback_phased-host
sanitize: throttle_feedback_phased-sanitize
.PHONY: c64-feedback
c64-feedback: throttle_feedback_phased-c64
