VELOCITY_CORE = $(GRAVITY_CORE) src/guidance_vector.c src/guidance_product.c src/guidance_shift.c src/guidance_velocity.c
VELOCITY_INPUT = $(GRAVITY_INPUT) tests/velocity_cases.c
VELOCITY_TEST = $(VELOCITY_INPUT) tests/velocity_check.c tests/frozen_words.c
VELOCITY_HEADERS = $(GRAVITY_HEADERS) include/guidance_velocity.h include/guidance_vector.h include/guidance_product.h include/guidance_shift.h tests/velocity_cases.h tests/velocity_vectors.h tests/velocity_vectors_all.h tests/velocity_vectors_a.h tests/velocity_vectors_b.h
all: build/velocity-snapshot build/test-velocity
build/velocity-snapshot: $(VELOCITY_CORE) $(VELOCITY_INPUT) platform/host/velocity_snapshot.c $(VELOCITY_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(VELOCITY_CORE) $(VELOCITY_INPUT) platform/host/velocity_snapshot.c -o $@
build/test-velocity: $(VELOCITY_CORE) $(VELOCITY_TEST) tests/test_velocity.c $(VELOCITY_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(VELOCITY_CORE) $(VELOCITY_TEST) tests/test_velocity.c -o $@
test: test-velocity
.PHONY: test-velocity sanitize-velocity c64-velocity
test-velocity: build/test-velocity
	./build/test-velocity
sanitize: sanitize-velocity
sanitize-velocity: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(VELOCITY_CORE) $(VELOCITY_TEST) tests/test_velocity.c -o build/test-velocity-sanitize
	./build/test-velocity-sanitize
c64-velocity: build/velocity-a.prg build/velocity-b.prg
build/velocity-a.prg: $(VELOCITY_CORE) $(VELOCITY_TEST) platform/c64/velocity_diagnostic.c $(VELOCITY_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -DVELOCITY_DIAGNOSTIC_HALF=0 -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/velocity-a.map $(VELOCITY_CORE) $(VELOCITY_TEST) platform/c64/velocity_diagnostic.c -o $@
build/velocity-b.prg: $(VELOCITY_CORE) $(VELOCITY_TEST) platform/c64/velocity_diagnostic.c $(VELOCITY_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -DVELOCITY_DIAGNOSTIC_HALF=1 -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/velocity-b.map $(VELOCITY_CORE) $(VELOCITY_TEST) platform/c64/velocity_diagnostic.c -o $@
