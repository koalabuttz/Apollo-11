# SPDX-License-Identifier: MIT
# Standalone milestone build; the parent owns integration into the main Makefile.
TF_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
TF_CORE = src/guidance_throttle.c src/guidance_product.c src/guidance_vector.c src/guidance_unit.c src/guidance_sum.c src/guidance_norm.c src/guidance_dp_divide.c src/agc_word.c src/agc_multiword.c tests/scenarios.c tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/throttle_feedback_cases.c
TF_TEST = tests/test_throttle_feedback.c tests/frozen_words.c $(TF_CORE)
.PHONY: throttle_feedback-host throttle_feedback-sanitize throttle_feedback-c64
throttle_feedback-host:
	gcc -O2 -Wconversion -pedantic $(TF_FLAGS) $(TF_TEST) -o build/test-throttle_feedback
	build/test-throttle_feedback
throttle_feedback-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(TF_FLAGS) $(TF_TEST) -o build/test-throttle_feedback-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-throttle_feedback-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(TF_FLAGS) tools/throttle_feedback_snapshot.c $(TF_CORE) -o build/throttle_feedback-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/throttle_feedback-native-sanitize > build/throttle_feedback-sanitize.snapshots
	cmp build/throttle_feedback-sanitize.snapshots build/throttle_feedback-reference.snapshots
throttle_feedback-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DTHROTTLE_FEEDBACK_CHECK_ONLY $(TF_FLAGS) tools/throttle_feedback_diagnostic.c $(TF_TEST) -Wl,-Map,build/throttle_feedback.map -o build/throttle_feedback.prg

test: throttle_feedback-host
sanitize: throttle_feedback-sanitize
