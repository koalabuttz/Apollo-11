# SPDX-License-Identifier: MIT
include tools/p66_launch_phased.mk
all: build/p66-launch-phased-port
build/p66-launch-phased-port: $(P66_LAUNCH_PHASED_CORE) tests/p66_bias_cases.c tests/p66_vertical_start_cases.c tests/p66_launch_cases.c tools/p66_launch_phased_snapshot.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-p66-launch-phased
test-p66-launch-phased:
	python3 tools/p66_launch_phased_differential.py
sanitize: sanitize-p66-launch-phased
sanitize-p66-launch-phased:
	python3 tools/p66_launch_phased_differential.py --sanitize
.PHONY: test-p66-launch-phased sanitize-p66-launch-phased
