# SPDX-License-Identifier: MIT
HORIZONTAL_DEFINITIONS = -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
HORIZONTAL_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/guidance_vector.c src/guidance_sum.c src/guidance_shift.c src/guidance_dp_divide.c src/guidance_norm.c src/guidance_unit.c src/guidance_horizontal.c
HORIZONTAL_INPUT = tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/horizontal_cases.c
HHP_CORE = $(HORIZONTAL_CORE) $(HORIZONTAL_INPUT) src/guidance_unit_phased.c src/guidance_length_phased.c src/guidance_horizontal_phased.c tests/horizontal_phased_cases.c
HHP_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests $(HORIZONTAL_DEFINITIONS)
all: build/horizontal-phased-native
build/horizontal-phased-native: $(HHP_CORE) tools/horizontal_phased_snapshot.c
	gcc -O2 $(HHP_FLAGS) tools/horizontal_phased_snapshot.c $(HHP_CORE) -o $@
test: all build/horizontal-phased-reference
	build/horizontal-phased-reference deps/fixture/MAIN.agc.bin > build/horizontal-phased-reference.snapshots 2> build/horizontal-phased-reference.trace
	build/horizontal-phased-native > build/horizontal-phased-native.snapshots
	cmp build/horizontal-phased-reference.snapshots build/horizontal-phased-native.snapshots
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(HHP_FLAGS) tools/horizontal_phased_snapshot.c $(HHP_CORE) -o build/horizontal-phased-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/horizontal-phased-sanitize > build/horizontal-phased-sanitize.snapshots
	cmp build/horizontal-phased-reference.snapshots build/horizontal-phased-sanitize.snapshots
.PHONY: all test sanitize

check-host:
	gcc -O2 $(HHP_FLAGS) tests/test_horizontal_phased.c tests/frozen_words.c $(HHP_CORE) -o build/test-horizontal-phased
	build/test-horizontal-phased
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DHORIZONTAL_PHASE_CHECK_ONLY -DHORIZONTAL_PHASE_SELECTED=$(if $(PHASE),$(PHASE),0) $(HHP_FLAGS) -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter tests/test_horizontal_phased.c tests/frozen_words.c tools/horizontal_phased_diagnostic.c $(HHP_CORE) -Wl,-Map,build/horizontal-phase$(if $(PHASE),$(PHASE),0).map -o build/horizontal-phase$(if $(PHASE),$(PHASE),0).prg

build/horizontal-phased-reference: tools/horizontal_phased_reference.c $(HORIZONTAL_INPUT)
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC $(HORIZONTAL_DEFINITIONS) -DHORIZONTAL_INPUT_ONLY tools/horizontal_phased_reference.c tests/scenarios.c $(HORIZONTAL_INPUT) src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
