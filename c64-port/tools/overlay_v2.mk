# SPDX-License-Identifier: MIT
# Overlay ABI 2 host checks. Standalone: make -f tools/overlay_v2.mk overlay-v2-test
# (run from c64-port/). Needs only a host C compiler and python3.
CC ?= cc
OV2_CFLAGS = -Iinclude -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic
OV2_SRC = src/overlay_runtime.c src/overlay_v2.c src/overlay_reu.c tests/test_overlay_v2.c
OV2_HDR = include/overlay_runtime.h include/overlay_v2.h include/overlay_reu.h
.PHONY: overlay-v2-test overlay-v2-plan
build/overlay-v2/selftest.reu: tools/overlay_v2_pack.py
	mkdir -p build/overlay-v2
	python3 tools/overlay_v2_pack.py --selftest build/overlay-v2/selftest --output $@
build/overlay-v2/test: $(OV2_SRC) $(OV2_HDR)
	mkdir -p build/overlay-v2
	$(CC) $(OV2_CFLAGS) -O2 -g $(OV2_SRC) -o $@
build/overlay-v2/test-sanitize: $(OV2_SRC) $(OV2_HDR)
	mkdir -p build/overlay-v2
	$(CC) $(OV2_CFLAGS) -O1 -g -fsanitize=address,undefined $(OV2_SRC) -o $@
overlay-v2-test: build/overlay-v2/selftest.reu build/overlay-v2/test build/overlay-v2/test-sanitize
	./build/overlay-v2/test build/overlay-v2/selftest.reu
	ASAN_OPTIONS=detect_leaks=0 ./build/overlay-v2/test-sanitize build/overlay-v2/selftest.reu
	python3 tools/overlay_v2_services.py --out-dir build/overlay-v2
	python3 tools/overlay_allocation.py --output build/overlay-v2/allocation.json
	cmp build/overlay-v2/allocation.json docs/overlay-allocation.json
overlay-v2-plan:
	python3 tools/overlay_allocation.py
