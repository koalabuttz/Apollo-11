PROJECTION_DEFINITIONS = -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
PROJECTION_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/guidance_trig.c src/guidance_dot.c src/guidance_dp_divide.c src/guidance_rotation.c src/guidance_projection.c
PROJECTION_INPUT = tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/trig_cases.c tests/projection_cases.c
PROJECTION_CHECK = tests/projection_check.c tests/frozen_words.c
PROJECTION_HEADERS = include/guidance_projection.h include/guidance_rotation.h include/guidance_trig.h include/guidance_norm.h include/guidance_product.h tests/projection_cases.h tests/projection_vectors.h tests/projection_vectors_all.h tests/projection_vectors_a.h tests/projection_vectors_b.h tests/projection_vectors_c.h tests/projection_vectors_d.h
all: build/test-projection build/projection-native
test: test-projection
sanitize: sanitize-projection
.PHONY: test-projection sanitize-projection c64-projection
build/test-projection: $(PROJECTION_CORE) $(PROJECTION_INPUT) $(PROJECTION_CHECK) tests/test_projection.c $(PROJECTION_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PROJECTION_DEFINITIONS) $(PROJECTION_CORE) $(PROJECTION_INPUT) $(PROJECTION_CHECK) tests/test_projection.c -o $@
build/projection-native: $(PROJECTION_CORE) $(PROJECTION_INPUT) tools/projection_snapshot.c $(PROJECTION_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PROJECTION_DEFINITIONS) $(PROJECTION_CORE) $(PROJECTION_INPUT) tools/projection_snapshot.c -o $@
test-projection: build/test-projection
	./build/test-projection
sanitize-projection: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(PROJECTION_DEFINITIONS) $(PROJECTION_CORE) $(PROJECTION_INPUT) $(PROJECTION_CHECK) tests/test_projection.c -o build/test-projection-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-projection-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(PROJECTION_DEFINITIONS) $(PROJECTION_CORE) $(PROJECTION_INPUT) tools/projection_snapshot.c -o build/projection-snapshot-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/projection-snapshot-sanitize > build/projection-sanitize.snapshots
	cmp build/projection-native.snapshots build/projection-sanitize.snapshots
c64-projection: build/projection-a.prg build/projection-b.prg build/projection-c.prg build/projection-d.prg
build/projection-%.prg: $(PROJECTION_CORE) $(PROJECTION_INPUT) $(PROJECTION_CHECK) platform/c64/projection_diagnostic.c $(PROJECTION_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) $(PROJECTION_DEFINITIONS) -DPROJECTION_C64_FROZEN_ENTRY -DPROJECTION_DIAGNOSTIC_PART=$(if $(filter a,$*),0,$(if $(filter b,$*),1,$(if $(filter c,$*),2,3))) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/projection-$*.map $(PROJECTION_CORE) $(PROJECTION_INPUT) $(PROJECTION_CHECK) platform/c64/projection_diagnostic.c -o $@

PROJECTION_PREFIX_CORE = $(RADIAL_CORE) src/guidance_acceleration.c src/guidance_trig.c src/guidance_rotation.c src/guidance_projection.c
PROJECTION_PREFIX_INPUT = $(RADIAL_INPUT) tests/unit_cases.c tests/trig_cases.c tests/projection_cases.c
all: build/projection-prefix-snapshot
build/projection-prefix-snapshot: $(PROJECTION_PREFIX_CORE) $(PROJECTION_PREFIX_INPUT) tools/projection_prefix_snapshot.c tests/acceleration_prefix.h $(PROJECTION_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PROJECTION_PREFIX_CORE) $(PROJECTION_PREFIX_INPUT) tools/projection_prefix_snapshot.c -o $@
sanitize-projection: sanitize-projection-prefix
.PHONY: sanitize-projection-prefix
sanitize-projection-prefix: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(PROJECTION_PREFIX_CORE) $(PROJECTION_PREFIX_INPUT) tools/projection_prefix_snapshot.c -o build/projection-prefix-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/projection-prefix-sanitize > build/projection-prefix-sanitize.snapshots
	cmp build/projection-prefix-native.snapshots build/projection-prefix-sanitize.snapshots
