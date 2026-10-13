# SPDX-License-Identifier: MIT
# Milestone t4rupt-core (lane A): the display and dispatch core of
# T4RUPT_PROGRAM.agc in both missions (lm/cm_t4rupt_core_routines).
T4C_SRC = src/t4rupt_core.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/t4rupt_core_cases.c
T4C_HDR = include/t4rupt_core.h include/agc_transfer.h include/agc_image_ops.h tests/t4rupt_core_cases.h
T4C_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-t4rupt-core sanitize-t4rupt-core c64-t4rupt-core vice-t4rupt-core t4rupt-core-differential
all: build/test-t4rupt-core
test: test-t4rupt-core
sanitize: sanitize-t4rupt-core
build/test-t4rupt-core: $(T4C_SRC) tests/test_t4rupt_core.c tests/t4rupt_core_vectors.h $(T4C_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(T4C_FLAGS) $(T4C_SRC) tests/test_t4rupt_core.c -o $@
test-t4rupt-core: build/test-t4rupt-core
	./build/test-t4rupt-core
sanitize-t4rupt-core: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(T4C_FLAGS) -fsanitize=address,undefined $(T4C_SRC) tests/test_t4rupt_core.c -o build/test-t4rupt-core-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-t4rupt-core-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
build/t4rupt-core-native: $(T4C_SRC) platform/host/t4rupt_core_snapshot.c $(T4C_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(T4C_FLAGS) $(T4C_SRC) platform/host/t4rupt_core_snapshot.c -o $@
t4rupt-core-differential: build/t4rupt-core-native
	python3 tools/t4rupt_core_freeze.py
	python3 tools/t4rupt_core_source_map.py
	git diff --exit-code -- tests/t4rupt_core_vectors.h tests/t4rupt_core_c64_vectors.h
c64-t4rupt-core: build/t4rupt-core.prg
build/t4rupt-core.prg: $(T4C_SRC) platform/c64/t4rupt_core_diagnostic.c tests/t4rupt_core_c64_vectors.h $(T4C_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(T4C_FLAGS) -Wl,-Map,build/t4rupt-core.map $(T4C_SRC) platform/c64/t4rupt_core_diagnostic.c -o $@
vice-t4rupt-core: build/t4rupt-core.prg
	python3 tools/vice_run.py build/t4rupt-core.prg --cycles 3000000000 --timeout 1200
