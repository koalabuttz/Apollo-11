# SPDX-License-Identifier: MIT
# Keep the standalone diagnostic selectors and recipes in their own make scope.
.PHONY: test-p66-fine-phases sanitize-p66-fine-phases
all: build/p66-fine-phases-host.stamp
build/p66-fine-phases-host.stamp: src/guidance_feedback_pairs.c src/guidance_length_phased.c src/guidance_horizontal_phased.c | build
	$(MAKE) -f tools/throttle_feedback_pairs.mk build/throttle-feedback-pairs-native
	$(MAKE) -f tools/horizontal_phased.mk build/horizontal-phased-native
	touch $@
test: test-p66-fine-phases
test-p66-fine-phases:
	$(MAKE) -f tools/throttle_feedback_pairs.mk check-host length-check-host
	$(MAKE) -f tools/horizontal_phased.mk check-host
sanitize: sanitize-p66-fine-phases
sanitize-p66-fine-phases:
	$(MAKE) -f tools/throttle_feedback_pairs.mk sanitize length-sanitize
	$(MAKE) -f tools/horizontal_phased.mk sanitize
