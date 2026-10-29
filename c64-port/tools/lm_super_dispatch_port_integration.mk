# SPDX-License-Identifier: MIT
.PHONY: lm-super-dispatch-port-test lm-super-dispatch-port-sanitize
lm-super-dispatch-port-test:
	python3 tools/lm_super_dispatch_port_host.py
lm-super-dispatch-port-sanitize:
	python3 tools/lm_super_dispatch_port_host.py --sanitize
