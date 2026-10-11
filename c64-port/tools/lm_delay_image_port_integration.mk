# SPDX-License-Identifier: MIT
.PHONY: lm-delay-image-port-test lm-delay-image-port-sanitize
lm-delay-image-port-test:
	python3 tools/lm_delay_image_port_host.py
lm-delay-image-port-sanitize:
	python3 tools/lm_delay_image_port_host.py --sanitize
