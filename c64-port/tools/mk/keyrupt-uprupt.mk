# SPDX-License-Identifier: MIT
# Milestone keyrupt-uprupt (lane A): KEYRUPT_UPRUPT.agc of Luminary099 and
# Comanche055 as native routines (lm_keyrupt_routines, cm_keyrupt_routines).
KU_SRC = src/keyrupt_uprupt.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/keyrupt_uprupt_cases.c
KU_HDR = include/keyrupt_uprupt.h include/agc_transfer.h include/agc_image_ops.h tests/keyrupt_uprupt_cases.h
KU_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-keyrupt-uprupt sanitize-keyrupt-uprupt c64-keyrupt-uprupt vice-keyrupt-uprupt keyrupt-uprupt-differential
all: build/test-keyrupt-uprupt
test: test-keyrupt-uprupt
sanitize: sanitize-keyrupt-uprupt
build/test-keyrupt-uprupt: $(KU_SRC) tests/test_keyrupt_uprupt.c tests/keyrupt_uprupt_vectors.h $(KU_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(KU_FLAGS) $(KU_SRC) tests/test_keyrupt_uprupt.c -o $@
test-keyrupt-uprupt: build/test-keyrupt-uprupt
	./build/test-keyrupt-uprupt
sanitize-keyrupt-uprupt: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(KU_FLAGS) -fsanitize=address,undefined $(KU_SRC) tests/test_keyrupt_uprupt.c -o build/test-keyrupt-uprupt-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-keyrupt-uprupt-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
build/keyrupt-uprupt-native: $(KU_SRC) platform/host/keyrupt_uprupt_snapshot.c $(KU_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(KU_FLAGS) $(KU_SRC) platform/host/keyrupt_uprupt_snapshot.c -o $@
keyrupt-uprupt-differential: build/keyrupt-uprupt-native
	python3 tools/keyrupt_uprupt_freeze.py
	python3 tools/keyrupt_uprupt_source_map.py
	git diff --exit-code -- tests/keyrupt_uprupt_vectors.h tests/keyrupt_uprupt_c64_vectors.h
c64-keyrupt-uprupt: build/keyrupt-uprupt.prg
build/keyrupt-uprupt.prg: $(KU_SRC) platform/c64/keyrupt_uprupt_diagnostic.c tests/keyrupt_uprupt_c64_vectors.h tests/keyrupt_uprupt_c64_rows.h $(KU_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(KU_FLAGS) -Wl,-Map,build/keyrupt-uprupt.map $(KU_SRC) platform/c64/keyrupt_uprupt_diagnostic.c -o $@
vice-keyrupt-uprupt: build/keyrupt-uprupt.prg
	python3 tools/vice_run.py build/keyrupt-uprupt.prg --cycles 3000000000 --timeout 1200
