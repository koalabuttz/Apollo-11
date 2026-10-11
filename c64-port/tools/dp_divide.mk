DP_DIVIDE_CORE = src/agc_word.c src/agc_multiword.c src/guidance_dp_divide.c
DP_DIVIDE_HEADERS = include/guidance_dp_divide.h include/guidance_divide.h tests/dp_divide_cases.h tests/dp_divide_vectors.h tests/frozen_words.h
all: build/dp-divide-native build/test-dp-divide
build/dp-divide-native: $(DP_DIVIDE_CORE) tests/dp_divide_snapshot.c $(DP_DIVIDE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DP_DIVIDE_CORE) tests/dp_divide_snapshot.c -o $@
build/test-dp-divide: $(DP_DIVIDE_CORE) tests/test_dp_divide.c tests/frozen_words.c $(DP_DIVIDE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DP_DIVIDE_CORE) tests/test_dp_divide.c tests/frozen_words.c -o $@
test: test-dp-divide
.PHONY: test-dp-divide sanitize-dp-divide c64-dp-divide
test-dp-divide: build/test-dp-divide
	./build/test-dp-divide
sanitize: sanitize-dp-divide
sanitize-dp-divide: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(DP_DIVIDE_CORE) tests/test_dp_divide.c tests/frozen_words.c -o build/test-dp-divide-sanitize
	./build/test-dp-divide-sanitize
c64-dp-divide: build/dp-divide.prg
build/dp-divide.prg: $(DP_DIVIDE_CORE) tests/test_dp_divide.c tests/frozen_words.c tools/dp_divide_diagnostic.c $(DP_DIVIDE_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -DDP_DIVIDE_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/dp-divide.map $(DP_DIVIDE_CORE) tests/test_dp_divide.c tests/frozen_words.c tools/dp_divide_diagnostic.c -o $@
