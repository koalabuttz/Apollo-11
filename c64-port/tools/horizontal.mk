HORIZONTAL_DEFINITIONS = -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
HORIZONTAL_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/guidance_vector.c src/guidance_sum.c src/guidance_shift.c src/guidance_dp_divide.c src/guidance_norm.c src/guidance_unit.c src/guidance_horizontal.c
HORIZONTAL_INPUT = tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/horizontal_cases.c
HORIZONTAL_CHECK = tests/horizontal_check.c tests/frozen_words.c
HORIZONTAL_HEADERS = include/guidance_horizontal.h include/guidance_norm.h include/guidance_product.h tests/horizontal_cases.h tests/horizontal_vectors.h tests/horizontal_vectors_all.h tests/horizontal_vectors_a.h tests/horizontal_vectors_b.h tests/horizontal_vectors_c.h tests/horizontal_vectors_d.h
all: build/test-horizontal build/horizontal-native
test: test-horizontal
sanitize: sanitize-horizontal
.PHONY: test-horizontal sanitize-horizontal c64-horizontal
build/test-horizontal: $(HORIZONTAL_CORE) $(HORIZONTAL_INPUT) $(HORIZONTAL_CHECK) tests/test_horizontal.c $(HORIZONTAL_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(HORIZONTAL_DEFINITIONS) $(HORIZONTAL_CORE) $(HORIZONTAL_INPUT) $(HORIZONTAL_CHECK) tests/test_horizontal.c -o $@
build/horizontal-native: $(HORIZONTAL_CORE) $(HORIZONTAL_INPUT) tools/horizontal_snapshot.c $(HORIZONTAL_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(HORIZONTAL_DEFINITIONS) $(HORIZONTAL_CORE) $(HORIZONTAL_INPUT) tools/horizontal_snapshot.c -o $@
test-horizontal: build/test-horizontal
	./build/test-horizontal
sanitize-horizontal: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(HORIZONTAL_DEFINITIONS) $(HORIZONTAL_CORE) $(HORIZONTAL_INPUT) $(HORIZONTAL_CHECK) tests/test_horizontal.c -o build/test-horizontal-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-horizontal-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(HORIZONTAL_DEFINITIONS) $(HORIZONTAL_CORE) $(HORIZONTAL_INPUT) tools/horizontal_snapshot.c -o build/horizontal-snapshot-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/horizontal-snapshot-sanitize > build/horizontal-sanitize.snapshots
	cmp build/horizontal-native.snapshots build/horizontal-sanitize.snapshots
c64-horizontal: build/horizontal-a.prg build/horizontal-b.prg build/horizontal-c.prg build/horizontal-d.prg
build/horizontal-%.prg: $(HORIZONTAL_CORE) $(HORIZONTAL_INPUT) $(HORIZONTAL_CHECK) platform/c64/horizontal_diagnostic.c $(HORIZONTAL_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) $(HORIZONTAL_DEFINITIONS) -DHORIZONTAL_C64_FROZEN_ENTRY -DHORIZONTAL_DIAGNOSTIC_PART=$(if $(filter a,$*),0,$(if $(filter b,$*),1,$(if $(filter c,$*),2,3))) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/horizontal-$*.map $(HORIZONTAL_CORE) $(HORIZONTAL_INPUT) $(HORIZONTAL_CHECK) platform/c64/horizontal_diagnostic.c -o $@
