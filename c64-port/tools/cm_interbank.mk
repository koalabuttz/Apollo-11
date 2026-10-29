# SPDX-License-Identifier: MIT
CM_INTERBANK_CORE = src/cm_interbank.c src/fixed_data.c src/cm_native_image.c src/cm_waitlist.c src/agc_word.c tests/cm_interbank_cases.c tests/cm_waitlist_image_cases.c
build/cm-interbank-native: $(CM_INTERBANK_CORE) platform/host/cm_interbank_snapshot.c include/cm_interbank.h tests/cm_interbank_cases.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DAPOLLO_FIXED_CM_ONLY $(CM_INTERBANK_CORE) platform/host/cm_interbank_snapshot.c -o $@
cm-interbank-differential: build/cm-interbank-native
	python3 tools/cm_interbank_differential.py

test: test-cm-interbank
test-cm-interbank: build/test-cm-interbank
	./build/test-cm-interbank
build/test-cm-interbank: $(CM_INTERBANK_CORE) tests/cm_interbank_check.c tests/test_cm_interbank.c tests/cm_interbank_vectors.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DAPOLLO_FIXED_CM_ONLY $(CM_INTERBANK_CORE) tests/cm_interbank_check.c tests/test_cm_interbank.c -o $@
sanitize: sanitize-cm-interbank
sanitize-cm-interbank: | build
	$(CC) $(CPPFLAGS) -DAPOLLO_FIXED_CM_ONLY -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_INTERBANK_CORE) tests/cm_interbank_check.c tests/test_cm_interbank.c -o build/test-cm-interbank-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-interbank-sanitize
c64-cm-interbank: build/cm-interbank.prg
build/cm-interbank.prg: $(CM_INTERBANK_CORE) tests/cm_interbank_check.c platform/c64/cm_interbank_diagnostic.c tests/cm_interbank_vectors.h | build
	$(MOS_CC) $(CPPFLAGS) -DAPOLLO_FIXED_CM_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/cm-interbank.map $(CM_INTERBANK_CORE) tests/cm_interbank_check.c platform/c64/cm_interbank_diagnostic.c -o $@
