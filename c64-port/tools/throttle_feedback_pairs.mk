# SPDX-License-Identifier: MIT
TFP_FLAGS = -DTHROTTLE_FEEDBACK_INPUT_ONLY -std=c99 -Wall -Wextra -Werror -Iinclude -Itests -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
TFP_CORE = src/guidance_feedback_pairs.c src/guidance_product.c src/guidance_vector.c src/guidance_unit.c src/guidance_sum.c src/guidance_shift.c src/guidance_gravity.c src/guidance_norm.c src/guidance_dp_divide.c src/agc_word.c src/agc_multiword.c tests/scenarios.c tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/throttle_feedback_cases.c tests/throttle_feedback_pairs_cases.c
all: build/throttle-feedback-pairs-native build/throttle-feedback-pairs-reference
build/throttle-feedback-pairs-native: $(TFP_CORE) tools/throttle_feedback_pairs_snapshot.c
	gcc -O2 -Wconversion -pedantic $(TFP_FLAGS) tools/throttle_feedback_pairs_snapshot.c $(TFP_CORE) -o $@
build/throttle-feedback-pairs-reference: tools/throttle_feedback_pairs_reference.c tests/throttle_feedback_pairs_cases.c
	gcc -O2 $(TFP_FLAGS) -DFEEDBACK_PAIRS_INPUT_ONLY -Ideps/virtualagc/yaAGC tools/throttle_feedback_pairs_reference.c $(filter tests/%,$(TFP_CORE)) src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/throttle-feedback-pairs-reference deps/fixture/MAIN.agc.bin > build/throttle-feedback-pairs-reference.snapshots 2> build/throttle-feedback-pairs-reference.trace
	build/throttle-feedback-pairs-native > build/throttle-feedback-pairs-native.snapshots
	cmp build/throttle-feedback-pairs-reference.snapshots build/throttle-feedback-pairs-native.snapshots
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wconversion -pedantic $(TFP_FLAGS) tools/throttle_feedback_pairs_snapshot.c $(TFP_CORE) -o build/throttle-feedback-pairs-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/throttle-feedback-pairs-sanitize > build/throttle-feedback-pairs-sanitize.snapshots
	cmp build/throttle-feedback-pairs-reference.snapshots build/throttle-feedback-pairs-sanitize.snapshots
.PHONY: all test sanitize

check-host:
	gcc -O2 -Wconversion -pedantic $(TFP_FLAGS) tests/test_throttle_feedback_pairs.c tests/frozen_words.c $(TFP_CORE) -o build/test-feedback-pairs
	build/test-feedback-pairs
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DFEEDBACK_PAIR_CHECK_ONLY -DFEEDBACK_PAIR_SELECTED=$(if $(PAIR),$(PAIR),0) $(TFP_FLAGS) -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter tests/test_throttle_feedback_pairs.c tests/frozen_words.c tools/throttle_feedback_pairs_diagnostic.c $(TFP_CORE) -Wl,-Map,build/feedback-pair$(if $(PAIR),$(PAIR),0).map -o build/feedback-pair$(if $(PAIR),$(PAIR),0).prg
length-test: build/feedback-length-reference
	build/feedback-length-reference deps/fixture/MAIN.agc.bin > build/feedback-length-reference.snapshots 2> build/feedback-length-reference.trace
	gcc -O2 -Wconversion -pedantic $(TFP_FLAGS) tools/feedback_length_snapshot.c src/guidance_length_phased.c $(TFP_CORE) -o build/feedback-length-native
	build/feedback-length-native > build/feedback-length-native.snapshots
	cmp build/feedback-length-reference.snapshots build/feedback-length-native.snapshots
length-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wconversion -pedantic $(TFP_FLAGS) tools/feedback_length_snapshot.c src/guidance_length_phased.c $(TFP_CORE) -o build/feedback-length-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/feedback-length-sanitize > build/feedback-length-sanitize.snapshots
	cmp build/feedback-length-reference.snapshots build/feedback-length-sanitize.snapshots

length-check-host:
	gcc -O2 -Wconversion -pedantic $(TFP_FLAGS) tests/test_feedback_length.c tests/frozen_words.c src/guidance_length_phased.c $(TFP_CORE) -o build/test-feedback-length
	build/test-feedback-length
length-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DFEEDBACK_LENGTH_CHECK_ONLY -DFEEDBACK_LENGTH_SELECTED=$(if $(PHASE),$(PHASE),0) $(TFP_FLAGS) -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter tests/test_feedback_length.c tests/frozen_words.c tools/feedback_length_diagnostic.c src/guidance_length_phased.c $(TFP_CORE) -Wl,-Map,build/feedback-length$(if $(PHASE),$(PHASE),0).map -o build/feedback-length$(if $(PHASE),$(PHASE),0).prg

build/feedback-length-reference: tools/feedback_length_reference.c $(filter tests/%,$(TFP_CORE))
	gcc -O2 $(TFP_FLAGS) -Wno-unused-variable -Wno-unused-but-set-variable -DFEEDBACK_PAIRS_INPUT_ONLY -Ideps/virtualagc/yaAGC tools/feedback_length_reference.c $(filter tests/%,$(TFP_CORE)) src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
