# SPDX-License-Identifier: MIT
.PHONY: lm-wake-image-port-test lm-wake-image-port-sanitize
lm-wake-image-port-test:
	python3 tools/lm_wake_image_port_host.py
lm-wake-image-port-sanitize:
	python3 tools/lm_wake_image_port_host.py --sanitize
