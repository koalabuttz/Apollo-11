# SPDX-License-Identifier: MIT
DSKY_MINUTES_OUTPUT_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_MINUTES_OUTPUT_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/dsky_pinball.c src/dsky_fraction_pair.c src/dsky_minutes_output.c tests/dsky_minutes_output_cases.c tests/dsky_minutes_output_check.c
all: build/test-dsky-minutes-output build/dsky-minutes-output-route-port
build/test-dsky-minutes-output: tests/test_dsky_minutes_output.c $(DSKY_MINUTES_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_MINUTES_OUTPUT_FLAGS) $^ -o $@
build/dsky-minutes-output-route-port: tools/dsky_minutes_output_snapshot.c $(DSKY_MINUTES_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_MINUTES_OUTPUT_FLAGS) $^ -o $@
dsky-minutes-output-host: build/test-dsky-minutes-output
	build/test-dsky-minutes-output
dsky-minutes-output-sanitize:
	python3 tools/dsky_minutes_output_sanitize.py
dsky-minutes-output-c64:
	python3 tools/dsky_minutes_output_build.py --mos-cc $(MOS_CC)
test: dsky-minutes-output-host
sanitize: dsky-minutes-output-sanitize
.PHONY: dsky-minutes-output-host dsky-minutes-output-sanitize dsky-minutes-output-c64
