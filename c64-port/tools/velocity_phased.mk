# SPDX-License-Identifier: MIT
VELOCITY_PHASED_CORE = $(VELOCITY_CORE) src/guidance_velocity_phased.c
VELOCITY_PHASED_INPUT = $(VELOCITY_INPUT) tests/velocity_phased_cases.c
VELOCITY_PHASED_C64_CORE = src/agc_word.c src/agc_multiword.c src/guidance_vector.c src/guidance_product.c src/guidance_shift.c src/guidance_velocity_phased.c
VELOCITY_PHASED_FLAGS = -DVELOCITY_INPUT_ONLY -DGRAVITY_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
.PHONY: velocity-phased-host velocity-phased-sanitize velocity-phased-c64
all: build/velocity-phased-snapshot
build/velocity-phased-snapshot: $(VELOCITY_PHASED_CORE) $(VELOCITY_PHASED_INPUT) tools/velocity_phased_snapshot.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(VELOCITY_PHASED_CORE) $(VELOCITY_PHASED_INPUT) tools/velocity_phased_snapshot.c -o $@
velocity-phased-host:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(VELOCITY_PHASED_FLAGS) $(VELOCITY_PHASED_C64_CORE) $(VELOCITY_PHASED_INPUT) tests/frozen_words.c tests/test_velocity_phased.c -o build/test-velocity-phased
	build/test-velocity-phased
velocity-phased-sanitize:
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(VELOCITY_PHASED_CORE) $(VELOCITY_PHASED_INPUT) tools/velocity_phased_snapshot.c -o build/velocity-phased-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/velocity-phased-sanitize > build/velocity-phased-sanitize.snapshots
	cmp build/velocity-phased-reference.snapshots build/velocity-phased-sanitize.snapshots
velocity-phased-c64:
	$(MOS_CC) $(CPPFLAGS) $(VELOCITY_PHASED_FLAGS) -DGUIDANCE_PHASE_CHECK_ONLY -DGUIDANCE_PHASE_SELECTED=$(if $(PHASE),$(PHASE),0) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(VELOCITY_PHASED_C64_CORE) $(VELOCITY_PHASED_INPUT) tests/frozen_words.c tests/test_velocity_phased.c tools/velocity_phased_diagnostic.c -Wl,-Map,build/velocity-phase$(if $(PHASE),$(PHASE),0).map -o build/velocity-phase$(if $(PHASE),$(PHASE),0).prg

test: velocity-phased-host
sanitize: velocity-phased-sanitize
