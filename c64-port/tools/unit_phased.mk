UNIT_PHASED_CORE = src/agc_word.c src/agc_multiword.c src/guidance_norm.c src/guidance_unit_phased.c
UNIT_PHASED_INPUT = tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/unit_phased_cases.c
UNIT_PHASED_FLAGS = -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
all: build/unit-phased-native
build/unit-phased-native: $(UNIT_PHASED_CORE) $(UNIT_PHASED_INPUT) tools/unit_phased_snapshot.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(UNIT_PHASED_FLAGS) $(UNIT_PHASED_CORE) $(UNIT_PHASED_INPUT) tools/unit_phased_snapshot.c -o $@
.PHONY: sanitize-unit-phased
sanitize: sanitize-unit-phased
sanitize-unit-phased:
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(UNIT_PHASED_FLAGS) $(UNIT_PHASED_CORE) $(UNIT_PHASED_INPUT) tools/unit_phased_snapshot.c -o build/unit-phased-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/unit-phased-sanitize > build/unit-phased-sanitize.snapshots
	cmp build/unit-phased-native.snapshots build/unit-phased-sanitize.snapshots
