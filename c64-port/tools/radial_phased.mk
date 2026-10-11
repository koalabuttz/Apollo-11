RADIAL_PHASED_CORE = $(RADIAL_CORE) src/guidance_unit_phased.c src/guidance_radial_phased.c
RADIAL_PHASED_INPUT = $(RADIAL_INPUT) tests/radial_phased_cases.c
all: build/radial-phased-snapshot
build/radial-phased-snapshot: $(RADIAL_PHASED_CORE) $(RADIAL_PHASED_INPUT) tools/radial_phased_snapshot.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RADIAL_PHASED_CORE) $(RADIAL_PHASED_INPUT) tools/radial_phased_snapshot.c -o $@
.PHONY: sanitize-radial-phased
sanitize: sanitize-radial-phased
sanitize-radial-phased:
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(RADIAL_PHASED_CORE) $(RADIAL_PHASED_INPUT) tools/radial_phased_snapshot.c -o build/radial-phased-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/radial-phased-sanitize > build/radial-phased-sanitize.snapshots
	cmp build/radial-phased-native.snapshots build/radial-phased-sanitize.snapshots
