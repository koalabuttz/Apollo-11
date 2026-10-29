# SPDX-License-Identifier: MIT
# Independent source phase-entry diagnostics; parent owns shared integration.
GP_FLAGS = -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY -DPROJECTION_INPUT_ONLY -DRADIAL_INPUT_ONLY -DVELOCITY_INPUT_ONLY -DGRAVITY_INPUT_ONLY
GP_BASE_INPUT = tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c
GP_unit_CORE = $(UNIT_PHASED_CORE)
GP_unit_INPUT = $(GP_BASE_INPUT) tests/unit_cases.c tests/unit_phased_cases.c
GP_radial_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/guidance_norm.c src/guidance_unit_phased.c src/guidance_dot.c src/guidance_shift.c src/guidance_sum.c src/guidance_radial_phased.c
GP_radial_INPUT = $(GP_BASE_INPUT) tests/gravity_cases.c tests/velocity_cases.c tests/radial_cases.c tests/radial_phased_cases.c
GP_projection_CORE = $(PROJECTION_PHASED_CORE)
GP_projection_INPUT = $(PROJECTION_PHASED_INPUT)
GP_CORE = $(GP_$(KIND)_CORE)
GP_TEST = $(GP_$(KIND)_INPUT) tests/frozen_words.c tests/test_$(KIND)_phased.c
.PHONY: guidance-phased-host guidance-phased-c64
guidance-phased-host:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GP_FLAGS) $(GP_CORE) $(GP_TEST) -o build/test-$(KIND)-phased
	build/test-$(KIND)-phased
guidance-phased-c64:
	$(MOS_CC) $(CPPFLAGS) $(GP_FLAGS) -DGUIDANCE_PHASE_CHECK_ONLY -DGUIDANCE_PHASE_SELECTED=$(PHASE) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(GP_CORE) $(GP_TEST) tools/$(KIND)_phased_diagnostic.c -Wl,-Map,build/$(KIND)-phase$(PHASE).map -o build/$(KIND)-phase$(PHASE).prg
