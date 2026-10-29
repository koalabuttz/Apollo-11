RADIAL_CORE = $(VELOCITY_CORE) src/guidance_dp_divide.c src/guidance_norm.c src/guidance_unit.c src/guidance_dot.c src/guidance_sum.c src/guidance_radial.c
RADIAL_INPUT = $(VELOCITY_INPUT) tests/radial_cases.c
RADIAL_TEST = $(RADIAL_INPUT) tests/radial_check.c tests/frozen_words.c
RADIAL_HEADERS = $(VELOCITY_HEADERS) include/guidance_unit.h include/guidance_norm.h include/guidance_dot.h include/guidance_sum.h include/guidance_dp_divide.h include/guidance_radial.h tests/radial_cases.h tests/radial_vectors.h tests/radial_vectors_all.h tests/radial_vectors_a.h tests/radial_vectors_b.h tests/radial_vectors_c.h tests/radial_vectors_d.h tests/radial_vectors_e.h tests/radial_vectors_f.h
all: build/radial-snapshot build/test-radial
test: test-radial
sanitize: sanitize-radial
build/radial-snapshot: $(RADIAL_CORE) $(RADIAL_INPUT) tools/radial_snapshot.c $(RADIAL_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RADIAL_CORE) $(RADIAL_INPUT) tools/radial_snapshot.c -o $@
build/test-radial: $(RADIAL_CORE) $(RADIAL_TEST) tests/test_radial.c $(RADIAL_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RADIAL_CORE) $(RADIAL_TEST) tests/test_radial.c -o $@
.PHONY: test-radial sanitize-radial c64-radial
test-radial: build/test-radial
	./build/test-radial
sanitize-radial: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(RADIAL_CORE) $(RADIAL_TEST) tests/test_radial.c -o build/test-radial-sanitize
	./build/test-radial-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(RADIAL_CORE) $(RADIAL_INPUT) tools/radial_snapshot.c -o build/radial-snapshot-sanitize
	./build/radial-snapshot-sanitize > build/radial-sanitize.snapshots
	cmp build/radial-native.snapshots build/radial-sanitize.snapshots
RADIAL_C64_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/guidance_dp_divide.c src/guidance_norm.c src/guidance_unit.c src/guidance_dot.c src/guidance_shift.c src/guidance_sum.c src/guidance_radial.c
RADIAL_C64_FLAGS = -DRADIAL_C64_FROZEN_ENTRY -DVELOCITY_INPUT_ONLY -DGRAVITY_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
c64-radial: build/radial-a.prg build/radial-b.prg build/radial-c.prg build/radial-d.prg build/radial-e.prg build/radial-f.prg
build/radial-%.prg: $(RADIAL_C64_CORE) $(RADIAL_TEST) platform/c64/radial_diagnostic.c $(RADIAL_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) $(RADIAL_C64_FLAGS) -DRADIAL_DIAGNOSTIC_HALF=$(if $(filter a,$*),0,$(if $(filter b,$*),1,$(if $(filter c,$*),2,$(if $(filter d,$*),3,$(if $(filter e,$*),4,5))))) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/radial-$*.map $(RADIAL_C64_CORE) $(RADIAL_TEST) platform/c64/radial_diagnostic.c -o $@
