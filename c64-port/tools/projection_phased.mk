PROJECTION_PHASED_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/guidance_trig.c src/guidance_dot.c src/guidance_rotation.c src/guidance_projection_phased.c
PROJECTION_PHASED_INPUT = tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/trig_cases.c tests/projection_cases.c tests/projection_phased_cases.c
PROJECTION_PHASED_FLAGS = -DPROJECTION_INPUT_ONLY -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
all: build/projection-phased-native
build/projection-phased-native: $(PROJECTION_PHASED_CORE) $(PROJECTION_PHASED_INPUT) tools/projection_phased_snapshot.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PROJECTION_PHASED_FLAGS) $(PROJECTION_PHASED_CORE) $(PROJECTION_PHASED_INPUT) tools/projection_phased_snapshot.c -o $@
.PHONY: sanitize-projection-phased
sanitize: sanitize-projection-phased
sanitize-projection-phased:
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(PROJECTION_PHASED_FLAGS) $(PROJECTION_PHASED_CORE) $(PROJECTION_PHASED_INPUT) tools/projection_phased_snapshot.c -o build/projection-phased-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/projection-phased-sanitize > build/projection-phased-sanitize.snapshots
	cmp build/projection-phased-native.snapshots build/projection-phased-sanitize.snapshots
