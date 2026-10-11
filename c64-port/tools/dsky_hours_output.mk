# SPDX-License-Identifier: MIT
DSKY_HOURS_OUTPUT_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_HOURS_OUTPUT_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/dsky_pinball.c src/dsky_decimal_raw.c src/dsky_read_pair.c src/dsky_hours_output.c tests/dsky_hours_output_cases.c tests/dsky_hours_output_check.c
all: build/test-dsky-hours-output build/dsky-hours-output-route-port
build/test-dsky-hours-output: tests/test_dsky_hours_output.c $(DSKY_HOURS_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_HOURS_OUTPUT_FLAGS) $^ -o $@
build/dsky-hours-output-route-port: tools/dsky_hours_output_snapshot.c $(DSKY_HOURS_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_HOURS_OUTPUT_FLAGS) $^ -o $@
dsky-hours-output-host: build/test-dsky-hours-output
	build/test-dsky-hours-output
dsky-hours-output-sanitize:
	python3 tools/dsky_hours_output_sanitize.py
dsky-hours-output-c64:
	python3 tools/dsky_hours_output_build.py --mos-cc $(MOS_CC)
test: dsky-hours-output-host
sanitize: dsky-hours-output-sanitize
.PHONY: dsky-hours-output-host dsky-hours-output-sanitize dsky-hours-output-c64
