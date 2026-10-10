CC = cc
MOS_CC ?= mos-c64-clang
CFLAGS ?= -O2 -g -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic
CPPFLAGS = -Iinclude -Itests
CORE = src/agc_word.c src/executive.c
SCENARIOS = tests/scenarios.c
HEADERS = include/agc_word.h include/executive.h tests/scenarios.h
.PHONY: all test c64 sanitize clean differential
all: build/test-host build/snapshot
build:
	mkdir -p build
build/test-host: $(CORE) $(SCENARIOS) tests/test_host.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(SCENARIOS) tests/test_host.c -o $@
build/snapshot: $(CORE) $(SCENARIOS) platform/host/snapshot.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORE) $(SCENARIOS) platform/host/snapshot.c -o $@
test: all
	./build/test-host
sanitize: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CORE) $(SCENARIOS) tests/test_host.c -o build/test-sanitize
	./build/test-sanitize
c64: build/executive.prg
build/executive.prg: $(CORE) $(SCENARIOS) platform/c64/main.c $(HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -Wall -Wextra -Werror -Wl,-Map,build/executive.map $(CORE) $(SCENARIOS) platform/c64/main.c -o $@
differential: all
	python3 tools/differential.py --reference "$(REFERENCE)"
clean:
	rm -rf build
