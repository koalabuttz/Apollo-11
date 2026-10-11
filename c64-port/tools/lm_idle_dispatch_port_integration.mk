# SPDX-License-Identifier: MIT
.PHONY: lm-idle-dispatch-port-test lm-idle-dispatch-port-sanitize
lm-idle-dispatch-port-test:
	python3 tools/lm_idle_dispatch_port_host.py
lm-idle-dispatch-port-sanitize:
	python3 tools/lm_idle_dispatch_port_host.py --sanitize
